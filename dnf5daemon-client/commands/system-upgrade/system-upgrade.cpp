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

#include "system-upgrade.hpp"

#include "commands/shared_options.hpp"
#include "context.hpp"
#include "exception.hpp"
#include "utils/auth.hpp"

#include <dnf5daemon-server/dbus.hpp>
#include <libdnf5/conf/option_string.hpp>

namespace dnfdaemon::client {

using namespace libdnf5::cli;

void SystemUpgradeCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}

void SystemUpgradeCommand::set_argument_parser() {
    auto & parser = get_context().get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cmd.set_description("prepare system for upgrade to a new release");

    auto no_downgrade = parser.add_new_named_arg("no_downgrade");
    no_downgrade->set_long_name("no-downgrade");
    no_downgrade->set_description(
        "Do not install packages from the new release if they are older than what is currently installed");
    no_downgrade->set_has_value(true);
    no_downgrade->set_arg_value_help("<yes|no>");
    no_downgrade->link_value(&no_downgrade_option);
    cmd.register_named_arg(no_downgrade);

    // TODO(mblaha): set the releasever named arg as required (currently no API for this)
}

void SystemUpgradeCommand::run() {
    auto & ctx = get_context();

    if (!libdnf5::utils::am_i_root()) {
        throw UnprivilegedUserError();
    }

    // TODO(mblaha): check the target releasever is set and different from the detected one

    dnfdaemon::KeyValueMap options = {};
    if (no_downgrade_option.get_value()) {
        options["mode"] = sdbus::Variant("upgrade");
    }

    ctx.session_proxy->callMethod("system_upgrade")
        .onInterface(dnfdaemon::INTERFACE_RPM)
        .withTimeout(static_cast<uint64_t>(-1))
        .withArguments(options);

    run_transaction(true);
}

}  // namespace dnfdaemon::client
