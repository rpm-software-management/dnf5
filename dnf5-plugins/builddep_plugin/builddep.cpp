/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "builddep.hpp"

#include "utils/string.hpp"

#include <dnf5/shared_options.hpp>
#include <libdnf5-cli/exception.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <rpm/rpmbuild.h>
#include <rpm/rpmds.h>
#include <rpm/rpmio.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmmacro.h>
#include <rpm/rpmts.h>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void BuildDepCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
}

void BuildDepCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Install build dependencies for package or spec file");

    auto specs = parser.add_new_positional_arg("specs", ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
    specs->set_description("List of specifications. Accepts *.spec / *.src.rpm files or package name.");
    specs->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            parse_builddep_specs(argc, argv);
            return true;
        });
    specs->set_complete_hook_func([&ctx](const char * arg) {
        return match_specs(ctx, arg, false, true, true, false, ".*\\.(spec|src\\.rpm|nosrc\\.rpm)");
    });
    cmd.register_positional_arg(specs);

    auto defs = parser.add_new_named_arg("rpm_macros");
    defs->set_short_name('D');
    defs->set_long_name("define");
    defs->set_has_value(true);
    defs->set_arg_value_help("\"macro expr\"");
    defs->set_description(
        "Define the RPM macro named \"macro\" to the value \"expr\" when parsing spec files. "
        "Does not apply for source rpm files.");
    defs->set_parse_hook_func(
        [this](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            auto split = libdnf5::utils::string::split(value, " ", 2);
            if (split.size() != 2) {
                throw libdnf5::cli::ArgumentParserError(
                    M_("Invalid value for macro definition \"{}\". \"macro expr\" format expected."),
                    std::string(value));
            }
            rpm_macros.emplace_back(std::move(split[0]), std::move(split[1]));
            return true;
        });
    cmd.register_named_arg(defs);

    allow_erasing = std::make_unique<AllowErasingOption>(*this);
    auto skip_unavailable = std::make_unique<SkipUnavailableOption>(*this);
    create_allow_downgrade_options(*this);
}

void BuildDepCommand::configure() {
    if (!pkg_specs.empty()) {
        get_context().base.get_repo_sack()->enable_source_repos();
    }

    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void BuildDepCommand::parse_builddep_specs(int specs_count, const char * const specs[]) {
    const std::string_view ext_spec(".spec");
    const std::string_view ext_srpm(".src.rpm");
    const std::string_view ext_nosrpm(".nosrc.rpm");
    std::set<std::string> unique_items;
    for (int i = 0; i < specs_count; ++i) {
        const std::string_view spec(specs[i]);
        if (auto [it, inserted] = unique_items.emplace(spec); inserted) {
            // TODO(mblaha): download remote URLs to temporary location + remove them afterwards
            if (spec.ends_with(ext_spec)) {
                spec_file_paths.emplace_back(spec);
            } else if (spec.ends_with(ext_srpm) || spec.ends_with(ext_nosrpm)) {
                srpm_file_paths.emplace_back(spec);
            } else {
                pkg_specs.emplace_back(spec);
            }
        }
    }
}

bool BuildDepCommand::add_from_spec_file(
    std::set<std::string> & install_specs, std::set<std::string> & conflicts_specs, const char * spec_file_name) {
    auto spec = rpmSpecParse(spec_file_name, RPMSPEC_ANYARCH | RPMSPEC_FORCE, nullptr);
    if (spec == nullptr) {
        std::cerr << "Failed to parse spec file \"" << spec_file_name << "\"." << std::endl;
        return false;
    }
    auto dependency_set = rpmdsInit(rpmSpecDS(spec, RPMTAG_REQUIRENAME));
    while (rpmdsNext(dependency_set) >= 0) {
        install_specs.emplace(rpmdsDNEVR(dependency_set) + 2);
    }
    rpmdsFree(dependency_set);
    auto conflicts_set = rpmdsInit(rpmSpecDS(spec, RPMTAG_CONFLICTNAME));
    while (rpmdsNext(conflicts_set) >= 0) {
        conflicts_specs.emplace(rpmdsDNEVR(conflicts_set) + 2);
    }
    rpmdsFree(conflicts_set);
    rpmSpecFree(spec);
    return true;
}

bool BuildDepCommand::add_from_srpm_file(
    std::set<std::string> & install_specs, std::set<std::string> & conflicts_specs, const char * srpm_file_name) {
    auto fd = Fopen(srpm_file_name, "r");
    if (fd == NULL || Ferror(fd)) {
        std::cerr << "Failed to open \"" << srpm_file_name << "\": " << Fstrerror(fd) << std::endl;
        if (fd) {
            Fclose(fd);
            fd = nullptr;
        }
        return false;
    }

    Header header;
    auto ts = rpmtsCreate();
    rpmtsSetVSFlags(ts, _RPMVSF_NOSIGNATURES | _RPMVSF_NODIGESTS);
    auto rc = rpmReadPackageFile(ts, fd, nullptr, &header);
    rpmtsFree(ts);
    Fclose(fd);
    fd = nullptr;

    if (rc == RPMRC_OK) {
        auto dependency_set = rpmdsInit(rpmdsNewPool(nullptr, header, RPMTAG_REQUIRENAME, 0));
        while (rpmdsNext(dependency_set) >= 0) {
            std::string_view reldep = rpmdsDNEVR(dependency_set) + 2;
            if (!reldep.starts_with("rpmlib(")) {
                install_specs.emplace(reldep);
            }
        }
        rpmdsFree(dependency_set);
        auto conflicts_set = rpmdsInit(rpmdsNewPool(nullptr, header, RPMTAG_CONFLICTNAME, 0));
        while (rpmdsNext(conflicts_set) >= 0) {
            conflicts_specs.emplace(rpmdsDNEVR(conflicts_set) + 2);
        }
        rpmdsFree(conflicts_set);
    } else {
        std::cerr << "Failed to read rpm file \"" << srpm_file_name << "\"." << std::endl;
    }

    headerFree(header);
    return true;
}

bool BuildDepCommand::add_from_pkg(
    std::set<std::string> & install_specs, std::set<std::string> & conflicts_specs, const std::string & pkg_spec) {
    auto & ctx = get_context();

    libdnf5::rpm::PackageQuery pkg_query(ctx.base);
    libdnf5::ResolveSpecSettings settings;
    settings.set_with_provides(false);
    settings.set_with_filenames(false);
    settings.set_with_binaries(false);
    settings.set_expand_globs(false);
    pkg_query.resolve_pkg_spec(pkg_spec, settings, false);

    std::vector<std::string> source_names{pkg_spec};
    for (const auto & pkg : pkg_query) {
        source_names.emplace_back(pkg.get_source_name());
    }

    libdnf5::rpm::PackageQuery source_pkgs(ctx.base);
    source_pkgs.filter_arch(std::vector<std::string>{"src", "nosrc"});
    source_pkgs.filter_name(source_names);
    if (source_pkgs.empty()) {
        std::cerr << "No package matched \"" << pkg_spec << "\"." << std::endl;
        return false;
    } else {
        for (const auto & pkg : source_pkgs) {
            for (const auto & reldep : pkg.get_requires()) {
                install_specs.emplace(reldep.to_string());
            }
            for (const auto & reldep : pkg.get_conflicts()) {
                conflicts_specs.emplace(reldep.to_string());
            }
        }
        return true;
    }
}

std::string escape_glob(const std::string & in) {
    // Escape fnmatch glob characters in a string
    std::string out;
    for (const auto ch : in) {
        if (ch == '*' || ch == '?' || ch == '[' || ch == ']' || ch == '\\') {
            out += '\\';
        }
        out += ch;
    }
    return out;
}

void BuildDepCommand::run() {
    // get build dependencies from various inputs
    std::set<std::string> install_specs{};
    std::set<std::string> conflicts_specs{};
    bool parse_ok = true;

    if (spec_file_paths.size() > 0) {
        for (const auto & macro : rpm_macros) {
            rpmPushMacro(nullptr, macro.first.c_str(), nullptr, macro.second.c_str(), -1);
        }

        for (const auto & spec : spec_file_paths) {
            parse_ok &= add_from_spec_file(install_specs, conflicts_specs, spec.c_str());
        }

        for (const auto & macro : rpm_macros) {
            rpmPopMacro(nullptr, macro.first.c_str());
        }
    } else {
        if (srpm_file_paths.size() > 0 && rpm_macros.size() > 0) {
            std::cerr << "Warning: -D or --define arguments have no meaning for source rpm packages." << std::endl;
        }
    }

    for (const auto & srpm : srpm_file_paths) {
        parse_ok &= add_from_srpm_file(install_specs, conflicts_specs, srpm.c_str());
    }

    for (const auto & pkg : pkg_specs) {
        parse_ok &= add_from_pkg(install_specs, conflicts_specs, pkg);
    }

    if (!parse_ok) {
        // failed to parse some of inputs (invalid spec, no package matched...)
        throw libdnf5::cli::Error(M_("Failed to parse some inputs."));
    }

    // fill the goal with build dependencies
    auto goal = get_context().get_goal();
    goal->set_allow_erasing(allow_erasing->get_value());

    // Search only for solution in provides and files. Use buildrequire with name search might result in inconsistent
    // behavior with installing dependencies of RPMs
    libdnf5::GoalJobSettings settings;
    settings.set_with_nevra(false);
    settings.set_with_binaries(false);

    // Don't expand globs in pkg specs. The special characters in a pkg spec
    // such as the brackets in `python3dist(build[virtualenv])`, should be
    // treated as literal.
    settings.expand_globs = false;

    for (const auto & spec : install_specs) {
        if (libdnf5::rpm::Reldep::is_rich_dependency(spec)) {
            goal->add_provide_install(spec);
        } else {
            // File provides could be satisfied by standard provides or files. With DNF5 we have to test both because
            // we do not download filelists and some files could be explicitly mentioned in provide section. The best
            // solution would be to merge result of provide and file search to prevent problems caused by modification
            // during distro lifecycle.

            // TODO(egoode) once we have a setting to disable expanding globs
            // in resolve_pkg_spec, escaping the glob characters will no longer
            // be needed:
            // https://github.com/rpm-software-management/dnf5/pull/1085
            const auto & escaped_spec = escape_glob(spec);
            goal->add_rpm_install(escaped_spec, settings);
        }
    }

    if (conflicts_specs.size() > 0) {
        auto & ctx = get_context();
        // exclude available (not installed) conflicting packages
        auto system_repo = ctx.base.get_repo_sack()->get_system_repo();
        auto rpm_package_sack = ctx.base.get_rpm_package_sack();
        libdnf5::rpm::PackageQuery conflicts_query_available(ctx.base);
        conflicts_query_available.filter_name(std::vector<std::string>{conflicts_specs.begin(), conflicts_specs.end()});
        libdnf5::rpm::PackageQuery conflicts_query_installed(conflicts_query_available);
        conflicts_query_available.filter_repo_id({system_repo->get_id()}, libdnf5::sack::QueryCmp::NEQ);
        rpm_package_sack->add_user_excludes(conflicts_query_available);

        // remove already installed conflicting packages
        conflicts_query_installed.filter_repo_id({system_repo->get_id()});
        goal->add_rpm_remove(conflicts_query_installed);
    }
}

void BuildDepCommand::goal_resolved() {
    auto & ctx = get_context();
    auto & transaction = *ctx.get_transaction();
    auto transaction_problems = transaction.get_problems();
    if (transaction_problems != libdnf5::GoalProblem::NO_PROBLEM) {
        auto skip_unavailable = ctx.base.get_config().get_skip_unavailable_option().get_value();
        if (transaction_problems != libdnf5::GoalProblem::NOT_FOUND || !skip_unavailable) {
            throw GoalResolveError(transaction);
        }
    }
}

}  // namespace dnf5
