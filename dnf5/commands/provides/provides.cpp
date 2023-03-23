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

#include "provides.hpp"

#include <libdnf/rpm/package_query.hpp>

#include <iostream>

namespace dnf5 {

using namespace libdnf::cli;

void ProvidesCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("query_commands").register_argument(arg_parser_this_cmd);
}

void ProvidesCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Find what package provides the given argument");

    auto keys = parser.add_new_positional_arg("specs", ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
    keys->set_description("List of package specs to query");
    keys->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                pkg_specs.emplace_back(argv[i]);
            }
            return true;
        });
    keys->set_complete_hook_func([&ctx](const char * arg) { return match_specs(ctx, arg, false, true, true, false); });
    cmd.register_positional_arg(keys);
}

void ProvidesCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void ProvidesCommand::run() {
    auto & ctx = get_context();

    libdnf::rpm::PackageSet result_pset(ctx.base);
    libdnf::rpm::PackageQuery full_package_query(ctx.base);
    auto provides_query = full_package_query;
    provides_query.filter_provides(pkg_specs, libdnf::sack::QueryCmp::GLOB);
    if (!provides_query.empty()) {
        full_package_query = provides_query;
    } else {
        // If provides query doesn't match anything try matching files
        full_package_query.filter_file(pkg_specs, libdnf::sack::QueryCmp::GLOB);
    }

    if (pkg_specs.empty()) {
        result_pset |= full_package_query;
    } else {
        const libdnf::ResolveSpecSettings settings{.ignore_case = true, .with_provides = false};
        for (const auto & spec : pkg_specs) {
            libdnf::rpm::PackageQuery package_query(full_package_query);
            package_query.resolve_pkg_spec(spec, settings, true);
            result_pset |= package_query;
        }
    }

    for (auto package : result_pset) {
        std::cout << package.get_full_nevra() << '\n';
    }
}


}  // namespace dnf5
