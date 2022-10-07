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

#include "distro-sync.hpp"

#include "context.hpp"
#include "exception.hpp"
#include "utils.hpp"

#include <dnf5daemon-server/dbus.hpp>
#include <libdnf/conf/option_string.hpp>

#include <iostream>
#include <memory>

namespace dnfdaemon::client {

using namespace libdnf::cli;

DistroSyncCommand::DistroSyncCommand(Context & context) : TransactionCommand(context, "distro-sync") {
    auto & parser = context.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cmd.set_description("Upgrade or downgrade installed software to the latest available versions");

    patterns_options = parser.add_new_values();
    auto patterns_arg = parser.add_new_positional_arg(
        "keys_to_match",
        libdnf::cli::ArgumentParser::PositionalArg::UNLIMITED,
        parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_options);
    patterns_arg->set_description("Patterns");
    cmd.register_positional_arg(patterns_arg);
}

void DistroSyncCommand::run() {
    auto & ctx = get_context();

    if (!am_i_root()) {
        throw UnprivilegedUserError();
    }

    // get package specs from command line and add them to the goal
    std::vector<std::string> patterns;
    if (patterns_options->size() > 0) {
        patterns.reserve(patterns_options->size());
        for (auto & pattern : *patterns_options) {
            auto option = dynamic_cast<libdnf::OptionString *>(pattern.get());
            patterns.emplace_back(option->get_value());
        }
    }

    dnfdaemon::KeyValueMap options = {};

    ctx.session_proxy->callMethod("distro_sync")
        .onInterface(dnfdaemon::INTERFACE_RPM)
        .withTimeout(static_cast<uint64_t>(-1))
        .withArguments(patterns, options);

    run_transaction();
}

}  // namespace dnfdaemon::client
