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

#include "changelog.hpp"

#include <libdnf5-cli/argument_parser.hpp>
#include <libdnf5-cli/output/changelogs.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/conf/option_string.hpp>
#include <libdnf5/rpm/package.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace dnf5 {

using namespace libdnf5::cli;

void ChangelogCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
}

void ChangelogCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Show package changelogs");

    since_option = dynamic_cast<libdnf5::OptionNumber<std::int64_t> *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionNumber<std::int64_t>>(
            new libdnf5::OptionNumber<int64_t>(0, [](const std::string & value) {
                struct tm time_m = {};
                std::istringstream ss(value);
                ss >> std::get_time(&time_m, "%Y-%m-%d");
                if (ss.fail()) {
                    throw libdnf5::cli::ArgumentParserError(
                        M_("Invalid date passed: \"{}\". Dates in \"YYYY-MM-DD\" format are expected"), value);
                }
                return static_cast<int64_t>(mktime(&time_m));
            }))));

    count_option = dynamic_cast<libdnf5::OptionNumber<std::int32_t> *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionNumber<std::int32_t>>(new libdnf5::OptionNumber<int>(0))));

    upgrades_option = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionBool>(new libdnf5::OptionBool(false))));

    auto since = parser.add_new_named_arg("since");
    since->set_long_name("since");
    since->set_description("Show changelog entries since date in the YYYY-MM-DD format");
    since->set_has_value(true);
    since->link_value(since_option);

    auto count = parser.add_new_named_arg("count");
    count->set_long_name("count");
    count->set_description("Limit the number of changelog entries shown per package");
    count->set_has_value(true);
    count->link_value(count_option);

    auto upgrades = parser.add_new_named_arg("upgrades");
    upgrades->set_long_name("upgrades");
    upgrades->set_description(
        "Show new changelog entries for packages that provide an upgrade for an already installed package");
    upgrades->set_const_value("true");
    upgrades->link_value(upgrades_option);

    pkgs_spec_to_show_options = parser.add_new_values();
    auto keys = parser.add_new_positional_arg(
        "pkg_spec",
        ArgumentParser::PositionalArg::UNLIMITED,
        parser.add_init_value(std::unique_ptr<libdnf5::Option>(new libdnf5::OptionString(nullptr))),
        pkgs_spec_to_show_options);
    keys->set_description("List of packages specifiers");
    keys->set_complete_hook_func([&ctx](const char * arg) { return match_specs(ctx, arg, false, true, false, false); });

    since->add_conflict_argument(*count);
    since->add_conflict_argument(*upgrades);
    count->add_conflict_argument(*upgrades);

    cmd.register_named_arg(since);
    cmd.register_named_arg(count);
    cmd.register_named_arg(upgrades);
    cmd.register_positional_arg(keys);
}

void ChangelogCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
    context.get_base().get_config().get_optional_metadata_types_option().add(
        libdnf5::Option::Priority::RUNTIME, libdnf5::OPTIONAL_METADATA_TYPES);
}

void ChangelogCommand::run() {
    auto & ctx = get_context();

    std::pair<libdnf5::cli::output::ChangelogFilterType, std::variant<libdnf5::rpm::PackageQuery, int64_t, int32_t>>
        filter = {libdnf5::cli::output::ChangelogFilterType::NONE, 0};
    libdnf5::rpm::PackageQuery full_package_query(ctx.get_base(), libdnf5::sack::ExcludeFlags::IGNORE_VERSIONLOCK);

    auto since = since_option->get_value();
    auto count = count_option->get_value();
    auto upgrades = upgrades_option->get_value();

    if (since > 0) {
        const auto since_time = static_cast<time_t>(since);
        filter = {libdnf5::cli::output::ChangelogFilterType::SINCE, since};
        std::cout << "Listing changelogs since " << std::put_time(std::localtime(&since_time), "%c") << std::endl;
    } else if (count != 0) {
        filter = {libdnf5::cli::output::ChangelogFilterType::COUNT, count};
        std::cout << "Listing only latest changelogs" << std::endl;
    } else if (upgrades) {
        filter = {libdnf5::cli::output::ChangelogFilterType::UPGRADES, full_package_query};
        std::cout << "Listing only new changelogs since installed version of the package" << std::endl;
    } else {
        std::cout << "Listing all changelogs" << std::endl;
    }

    //query
    libdnf5::rpm::PackageQuery query(ctx.get_base(), libdnf5::sack::ExcludeFlags::IGNORE_VERSIONLOCK, true);
    libdnf5::ResolveSpecSettings settings;
    settings.set_ignore_case(true);
    settings.set_with_nevra(true);
    settings.set_with_provides(false);
    settings.set_with_filenames(false);
    settings.set_with_binaries(false);
    if (pkgs_spec_to_show_options->size() > 0) {
        for (auto & pattern : *pkgs_spec_to_show_options) {
            libdnf5::rpm::PackageQuery package_query(full_package_query);
            auto option = dynamic_cast<libdnf5::OptionString *>(pattern.get());
            package_query.resolve_pkg_spec(option->get_value(), settings, true);
            package_query.filter_latest_evr();
            query |= package_query;
        }
    } else {
        query = full_package_query;
    }
    if (upgrades) {
        query.filter_upgrades();
    } else {
        query.filter_available();
    }

    libdnf5::cli::output::print_changelogs(query, filter);
}

}  // namespace dnf5
