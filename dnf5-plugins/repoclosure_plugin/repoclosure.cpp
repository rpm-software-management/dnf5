// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "repoclosure.hpp"

#include <libdnf5-cli/argument_parser.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/rpm/package.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/reldep.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/format.hpp>

#include <iostream>


namespace dnf5 {

void RepoclosureCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
}

void RepoclosureCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Print list of unresolved dependencies for repositories"));

    auto specs = parser.add_new_positional_arg(
        "specs", libdnf5::cli::ArgumentParser::PositionalArg::UNLIMITED, nullptr, nullptr);
    specs->set_description("List of package specs to check closure for");
    specs->set_parse_hook_func(
        [this](
            [[maybe_unused]] libdnf5::cli::ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                pkg_specs.emplace_back(argv[i]);
            }
            return true;
        });
    cmd.register_positional_arg(specs);

    auto * check_repos_arg = parser.add_new_named_arg("check");
    check_repos_arg->set_long_name("check");
    check_repos_arg->set_description("Specify repo ids to check");
    check_repos_arg->set_has_value(true);
    check_repos_arg->set_arg_value_help("<REPO_ID>,...");
    check_repos_arg->set_parse_hook_func([this](
                                             [[maybe_unused]] libdnf5::cli::ArgumentParser::NamedArg * arg,
                                             [[maybe_unused]] const char * option,
                                             const char * value) {
        libdnf5::OptionStringList list_value(value);
        for (const auto & repoid : list_value.get_value()) {
            check_repos.emplace_back(repoid);
        }
        return true;
    });
    cmd.register_named_arg(check_repos_arg);

    auto * arches_arg = parser.add_new_named_arg("arch");
    arches_arg->set_long_name("arch");
    arches_arg->set_description("Only check packages of specified architectures.");
    arches_arg->set_has_value(true);
    arches_arg->set_arg_value_help("<ARCH>,...");
    arches_arg->set_parse_hook_func([this](
                                        [[maybe_unused]] libdnf5::cli::ArgumentParser::NamedArg * arg,
                                        [[maybe_unused]] const char * option,
                                        const char * value) {
        libdnf5::OptionStringList list_value(value);
        for (const auto & arch : list_value.get_value()) {
            arches.emplace_back(arch);
        }
        return true;
    });
    cmd.register_named_arg(arches_arg);

    newest = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "newest", '\0', "Only consider the latest version of a package from each repo.", false);
}

void RepoclosureCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(false);
    // filelists needed because there are packages in repos with file requirements
    context.get_base().get_config().get_optional_metadata_types_option().add_item(
        libdnf5::Option::Priority::RUNTIME, libdnf5::METADATA_TYPE_FILELISTS);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void RepoclosureCommand::run() {
    auto & ctx = get_context();

    // available_query is the set of packages used to satisfy dependencies
    // to_check_query is the set of packages we are checking the dependencies of
    // in case that `--newest` was used, start with an empty query
    bool newest_used = newest->get_value();
    libdnf5::rpm::PackageQuery available_query(
        ctx.get_base(), libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES, newest_used);
    libdnf5::rpm::PackageQuery to_check_query(ctx.get_base(), libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES, newest_used);
    if (newest_used) {
        libdnf5::repo::RepoQuery repos(ctx.get_base());
        repos.filter_enabled(true);
        for (const auto & repo : repos) {
            libdnf5::rpm::PackageQuery repo_pkgs(ctx.get_base());
            repo_pkgs.filter_repo_id({repo->get_id()});
            repo_pkgs.filter_latest_evr();
            available_query |= repo_pkgs;
            to_check_query |= repo_pkgs;
        }
    }

    if (!check_repos.empty()) {
        to_check_query.filter_repo_id(check_repos, libdnf5::sack::QueryCmp::GLOB);
    }

    if (!arches.empty()) {
        to_check_query.filter_arch(arches);
    }

    if (ctx.get_base().get_config().get_best_option().get_value()) {
        available_query.filter_latest_evr();
    }

    if (!pkg_specs.empty()) {
        libdnf5::rpm::PackageQuery to_check_pkgs(ctx.get_base(), libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES, true);
        libdnf5::ResolveSpecSettings settings;
        settings.set_with_nevra(true);
        settings.set_with_provides(false);
        settings.set_with_filenames(false);
        settings.set_with_binaries(false);
        bool specs_resolved = true;
        for (const auto & spec : pkg_specs) {
            libdnf5::rpm::PackageQuery package_query(to_check_query);
            auto nevra_pair = package_query.resolve_pkg_spec(spec, settings, true);
            if (!nevra_pair.first) {
                specs_resolved = false;
                std::cerr << libdnf5::utils::sformat(_("No match for argument \"{}\"."), spec) << std::endl;
                continue;
            }
            to_check_pkgs |= package_query;
        }
        if (!specs_resolved) {
            throw libdnf5::cli::CommandExitError(1, M_("Failed to resolve package specifications."));
        }
        to_check_query = to_check_pkgs;
    }


    // cache query results: reldepid -> whether the reldep is satisfied
    std::unordered_map<int, bool> resolved;
    std::vector<std::pair<libdnf5::rpm::Package, std::vector<std::string>>> unresolved_packages;
    for (const auto & pkg : to_check_query) {
        std::vector<std::string> unsatisfied;
        for (const auto & reldep : pkg.get_requires()) {
            int reldep_id = reldep.get_id().id;
            auto resolved_it = resolved.find(reldep_id);
            bool satisfied;
            if (resolved_it == resolved.end()) {
                satisfied = available_query.is_dep_satisfied(reldep);
                resolved.emplace(reldep_id, satisfied);
            } else {
                satisfied = resolved_it->second;
            }
            if (!satisfied) {
                unsatisfied.emplace_back(reldep.to_string());
            }
        }
        if (!unsatisfied.empty()) {
            std::sort(unsatisfied.begin(), unsatisfied.end());
            unresolved_packages.emplace_back(pkg, std::move(unsatisfied));
        }
    }

    if (!unresolved_packages.empty()) {
        // sort unresolved packages
        std::sort(
            unresolved_packages.begin(),
            unresolved_packages.end(),
            [](const std::pair<libdnf5::rpm::Package, std::vector<std::string>> & l,
               const std::pair<libdnf5::rpm::Package, std::vector<std::string>> & r) {
                return libdnf5::rpm::cmp_nevra<libdnf5::rpm::Package>(l.first, r.first);
            });
        int unresolved_deps_count = 0;

        for (auto const & [pkg, unsatisfied] : unresolved_packages) {
            std::cout << "package: " << pkg.get_nevra() << " from " << pkg.get_repo_id() << std::endl;
            std::cout << "  unresolved deps (" << unsatisfied.size() << "):" << std::endl;
            for (const auto & dep : unsatisfied) {
                unresolved_deps_count += 1;
                std::cout << "    " << dep << std::endl;
            }
        }

        throw libdnf5::cli::CommandExitError(
            1,
            M_("Error: Repoclosure ended with unresolved dependencies ({}) across {} packages."),
            unresolved_deps_count,
            unresolved_packages.size());
    }
}

}  // namespace dnf5
