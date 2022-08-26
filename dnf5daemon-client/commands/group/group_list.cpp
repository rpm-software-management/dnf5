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

#include "group_list.hpp"

#include "context.hpp"
#include "wrappers/dbus_group_wrapper.hpp"

#include <libdnf-cli/output/groupinfo.hpp>
#include <libdnf-cli/output/grouplist.hpp>
#include <libdnf/conf/option.hpp>
#include <libdnf/conf/option_string.hpp>

#include <iostream>

namespace dnfdaemon::client {

using namespace libdnf::cli;

GroupListCommand::GroupListCommand(Command & parent, const char * command)
    : DaemonCommand(parent, command),
      command(command) {
    auto & ctx = static_cast<Context &>(get_session());
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cmd.set_description("search for packages matching keyword");

    patterns_options = parser.add_new_values();
    auto keys = parser.add_new_positional_arg(
        "keys_to_match",
        ArgumentParser::PositionalArg::UNLIMITED,
        parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_options);
    keys->set_description("List of keys to match");
    cmd.register_positional_arg(keys);
}

void GroupListCommand::run() {
    auto & ctx = static_cast<Context &>(get_session());

    // query groups
    dnfdaemon::KeyValueMap options = {};

    std::vector<std::string> patterns;
    if (patterns_options->size() > 0) {
        patterns.reserve(patterns_options->size());
        for (auto & pattern : *patterns_options) {
            auto option = dynamic_cast<libdnf::OptionString *>(pattern.get());
            patterns.emplace_back(option->get_value());
        }
    }
    options["patterns"] = patterns;

    std::vector<std::string> attributes{"groupid", "name", "installed"};
    if (command == "info") {
        std::vector<std::string> more_attributes{
            "description", "order", "langonly", "uservisible", "repos", "packages"};
        attributes.reserve(attributes.size() + more_attributes.size());
        std::move(std::begin(more_attributes), std::end(more_attributes), std::back_inserter(attributes));
    }
    options["attributes"] = attributes;

    dnfdaemon::KeyValueMapList raw_groups;
    ctx.session_proxy->callMethod("list")
        .onInterface(dnfdaemon::INTERFACE_GROUP)
        .withTimeout(static_cast<uint64_t>(-1))
        .withArguments(options)
        .storeResultsTo(raw_groups);

    std::vector<DbusGroupWrapper> groups{};
    for (auto & group : raw_groups) {
        groups.push_back(DbusGroupWrapper(group));
    }

    if (command == "info") {
        for (auto & group : groups) {
            libdnf::cli::output::print_groupinfo_table(group);
            std::cout << '\n';
        }
    } else {
        libdnf::cli::output::print_grouplist_table(groups);
    }
}

}  // namespace dnfdaemon::client
