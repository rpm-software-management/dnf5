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

#include "dnf5/context.hpp"
#include "utils/bgettext/bgettext-mark-domain.h"
#include "utils/string.hpp"

#include "libdnf-cli/output/transaction_table.hpp"

#include <libdnf/base/goal.hpp>
#include <libdnf/conf/option_string.hpp>
#include <libdnf/rpm/package_query.hpp>
#include <rpm/rpmbuild.h>

#include <iostream>


namespace fs = std::filesystem;


namespace dnf5 {


using namespace libdnf::cli;

BuildDepCommand::BuildDepCommand(Command & parent) : Command(parent, "builddep") {
    auto & ctx = static_cast<Context &>(get_session());
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("Install build dependencies for package or spec file");

    auto specs = parser.add_new_positional_arg("specs", ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
    specs->set_short_description("List of specifications. Accepts *.spec / *.src.rpm files or package name.");
    specs->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            parse_builddep_specs(argc, argv);
            return true;
        });
    specs->set_complete_hook_func([&ctx](const char * arg) {
        return match_specs(ctx, arg, false, true, true, false, ".*\\.(spec|src\\.rpm|nosrc\\.rpm)");
    });
    cmd.register_positional_arg(specs);

    auto skip_unavailable = parser.add_new_named_arg("skip-unavailable");
    skip_unavailable->set_long_name("skip-unavailable");
    skip_unavailable->set_short_description("Skip build dependencies not available in repositories");
    skip_unavailable->set_const_value("true");
    skip_unavailable->link_value(&skip_unavailable_option);
    cmd.register_named_arg(skip_unavailable);

    auto defs = parser.add_new_named_arg("rpm_macros");
    defs->set_short_name('D');
    defs->set_long_name("define");
    defs->set_has_value(true);
    defs->set_arg_value_help("\"macro expr\"");
    defs->set_short_description("Define the RPM macro named \"macro\" to the value \"expr\" when parsing spec files");
    defs->set_parse_hook_func(
        [this](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            auto split = libdnf::utils::string::split(value, " ", 2);
            if (split.size() != 2) {
                throw libdnf::cli::ArgumentParserError(
                    M_("Invalid value for macro definition \"{}\". \"macro expr\" format expected."), value);
            }
            rpm_macros.emplace_back(std::move(split[0]), std::move(split[1]));
            return true;
        });
    cmd.register_named_arg(defs);
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


bool BuildDepCommand::add_from_spec_file(std::set<std::string> & specs, const char * spec_file_name) {
    auto spec = rpmSpecParse(spec_file_name, RPMSPEC_ANYARCH | RPMSPEC_FORCE, nullptr);
    if (spec == nullptr) {
        std::cout << "Failed to parse spec file \"" << spec_file_name << "\"." << std::endl;
        return false;
    }
    auto dependency_set = rpmdsInit(rpmSpecDS(spec, RPMTAG_REQUIRENAME));
    while (rpmdsNext(dependency_set) >= 0) {
        specs.emplace(rpmdsDNEVR(dependency_set) + 2);
    }
    rpmdsFree(dependency_set);
    rpmSpecFree(spec);
    return true;
}


bool BuildDepCommand::add_from_srpm_file(std::set<std::string> & specs, const char * srpm_file_name) {
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
                specs.emplace(reldep);
            }
        }
        rpmdsFree(dependency_set);
    } else {
        std::cerr << "Failed to read rpm file \"" << srpm_file_name << "\"." << std::endl;
    }

    headerFree(header);
    return true;
}


bool BuildDepCommand::add_from_pkg(std::set<std::string> & specs, const std::string & pkg_spec) {
    auto & ctx = static_cast<Context &>(get_session());

    libdnf::rpm::PackageQuery pkg_query(ctx.base);
    pkg_query.resolve_pkg_spec(
        pkg_spec, libdnf::ResolveSpecSettings{.with_provides = false, .with_filenames = false}, false);

    std::vector<std::string> source_names{pkg_spec};
    for (const auto & pkg : pkg_query) {
        source_names.emplace_back(pkg.get_source_name());
    }

    libdnf::rpm::PackageQuery source_pkgs(ctx.base);
    source_pkgs.filter_arch({"src"});
    source_pkgs.filter_name(source_names);
    if (source_pkgs.empty()) {
        std::cerr << "No package matched \"" << pkg_spec << "\"." << std::endl;
        return false;
    } else {
        for (const auto & pkg : source_pkgs) {
            for (const auto & reldep : pkg.get_requires()) {
                specs.emplace(reldep.to_string());
            }
        }
        return true;
    }
}

void BuildDepCommand::run() {
    auto & ctx = static_cast<Context &>(get_session());

    if (!pkg_specs.empty()) {
        ctx.base.get_repo_sack()->enable_source_repos();
    }

    ctx.load_repos(true);

    // get build dependencies from various inputs
    std::set<std::string> install_specs{};
    bool parse_ok = true;

    if (spec_file_paths.size() > 0) {
        for (const auto & macro : rpm_macros) {
            rpmPushMacro(nullptr, macro.first.c_str(), nullptr, macro.second.c_str(), -1);
        }

        for (const auto & spec : spec_file_paths) {
            parse_ok &= add_from_spec_file(install_specs, spec.c_str());
        }

        for (const auto & macro : rpm_macros) {
            rpmPopMacro(nullptr, macro.first.c_str());
        }
    }

    for (const auto & srpm : srpm_file_paths) {
        parse_ok &= add_from_srpm_file(install_specs, srpm.c_str());
    }

    for (const auto & pkg : pkg_specs) {
        parse_ok &= add_from_pkg(install_specs, pkg);
    }

    if (!parse_ok) {
        // failed to parse some of inputs (invalid spec, no package matched...)
        // TODO(mblaha): command failed, throw an exception here
        return;
    }

    // fill the goal with build dependencies
    libdnf::Goal goal(ctx.base);
    for (const auto & spec : install_specs) {
        if (spec[0] == '(') {
            // rich dependencies require different method
            goal.add_provide_install(spec);
        } else {
            goal.add_rpm_install(spec);
        }
    }

    auto transaction = goal.resolve(false);
    auto skip_unavailable = skip_unavailable_option.get_value();
    auto transaction_problems = transaction.get_problems();
    if (transaction_problems != libdnf::GoalProblem::NO_PROBLEM) {
        if (transaction_problems != libdnf::GoalProblem::NOT_FOUND || !skip_unavailable) {
            throw GoalResolveError(transaction);
        }
    }

    if (!libdnf::cli::output::print_transaction_table(transaction)) {
        // the transaction was empty
        return;
    }

    if (!userconfirm(ctx.base.get_config())) {
        throw AbortedByUserError();
    }

    ctx.download_and_run(transaction);
}

}  // namespace dnf5
