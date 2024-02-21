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
#include "libdnf5-cli/output/provides.hpp"

#include "provides.hpp"

#include "libdnf5/common/sack/query_cmp.hpp"
#include "libdnf5/conf/option_string.hpp"

#include <libdnf5/conf/const.hpp>
#include <libdnf5/rpm/package_query.hpp>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void ProvidesCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}

void ProvidesCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Find what package provides the given value");

    auto * keys = parser.add_new_positional_arg("specs", ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
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
    context.base.get_config().get_optional_metadata_types_option().add_item(
        libdnf5::Option::Priority::RUNTIME, libdnf5::METADATA_TYPE_FILELISTS);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

std::pair<libdnf5::rpm::PackageQuery, libdnf5::cli::output::ProvidesMatchedBy> ProvidesCommand::filter_spec(
    std::string spec, const libdnf5::rpm::PackageQuery & full_package_query) {
    auto provides_query = full_package_query;
    auto filename_query = full_package_query;
    auto binary_query = full_package_query;

    // if the spec starts with "/" assume it's a provide by filename and skip this query
    if (!(spec.rfind("/", 0) == 0)) {
        provides_query.filter_provides(std::vector<std::string>({spec}), libdnf5::sack::QueryCmp::GLOB);
        if (!provides_query.empty()) {
            return std::make_pair(provides_query, libdnf5::cli::output::ProvidesMatchedBy::PROVIDES);
        }
        // if the spec is a binary name and there is no match from provide, try to prepend
        // /bin /sbin to match it by binary provides and continue
        else {
            std::string bin_spec = "/bin/" + spec;
            std::string sbin_spec = "/sbin/" + spec;
            std::string usr_bin_spec = "/usr/bin/" + spec;
            std::string usr_sbin_spec = "/usr/sbin/" + spec;
            binary_query.filter_file(
                std::vector<std::string>({bin_spec, sbin_spec, usr_bin_spec, usr_sbin_spec}),
                libdnf5::sack::QueryCmp::GLOB);
            if (!binary_query.empty()) {
                return std::make_pair(binary_query, libdnf5::cli::output::ProvidesMatchedBy::BINARY);
            }
        }
    }
    // compatibility for packages that didn't do UsrMove
    if ((spec.rfind("/bin/", 0) == 0) || (spec.rfind("/sbin/", 0) == 0)) {
        spec.insert(0, "/usr");
    }
    filename_query.filter_file(std::vector<std::string>({spec}), libdnf5::sack::QueryCmp::GLOB);
    if (!filename_query.empty()) {
        return std::make_pair(filename_query, libdnf5::cli::output::ProvidesMatchedBy::FILENAME);
    }
    return std::make_pair(full_package_query, libdnf5::cli::output::ProvidesMatchedBy::NO_MATCH);
}

void ProvidesCommand::run() {
    auto & ctx = get_context();
    bool any_match = false;
    std::set<std::string> unmatched_specs;

    for (auto & spec : pkg_specs) {
        libdnf5::rpm::PackageQuery full_package_query(ctx.base, libdnf5::sack::ExcludeFlags::IGNORE_VERSIONLOCK);
        // get the matched query first and the type of match (no_match, provides, file, binary) second
        auto matched = filter_spec(spec, full_package_query);
        for (auto package : matched.first) {
            if (matched.second != libdnf5::cli::output::ProvidesMatchedBy::NO_MATCH) {
                libdnf5::cli::output::print_provides_table(package, spec.c_str(), matched.second);
                any_match = true;
            } else {
                unmatched_specs.insert(spec);
            }
        }
    }
    if (!unmatched_specs.empty() && any_match) {
        for (auto const & spec : unmatched_specs) {
            std::cerr << "No matches found for " << spec << "." << std::endl;
        }
        std::cerr << "If searching for a file, try specifying the full "
                     "path or using a wildcard prefix (\"*/\") at the beginning."
                  << std::endl;
        throw libdnf5::cli::SilentCommandExitError(1);
    }
    if (!any_match) {
        std::cerr << "No matches found. If searching for a file, try specifying the full "
                     "path or using a wildcard prefix (\"*/\") at the beginning."
                  << std::endl;
        throw libdnf5::cli::SilentCommandExitError(1);
    }
}

}  // namespace dnf5
