// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "remove.hpp"

#include "commands/shared_options.hpp"
#include "context.hpp"
#include "exception.hpp"
#include "utils/auth.hpp"

#include <dnf5daemon-server/dbus.hpp>
#include <libdnf5/conf/option_string.hpp>

#include <memory>

namespace dnfdaemon::client {

using namespace libdnf5::cli;

void RemoveCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}

void RemoveCommand::set_argument_parser() {
    auto & context = get_context();
    auto & parser = context.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cmd.set_description("remove packages on the system");

    auto specs_arg = pkg_specs_argument(parser, libdnf5::cli::ArgumentParser::PositionalArg::AT_LEAST_ONE, pkg_specs);
    specs_arg->set_description("List of packages to remove");
    cmd.register_positional_arg(specs_arg);

    auto offline_arg = create_offline_option(parser, &offline_option);
    cmd.register_named_arg(offline_arg);

    // run remove command always with allow_erasing on
    context.allow_erasing.set(libdnf5::Option::Priority::RUNTIME, true);
}

void RemoveCommand::run() {
    auto & ctx = get_context();

    if (!libdnf5::utils::am_i_root()) {
        throw UnprivilegedUserError();
    }

    dnfdaemon::KeyValueMap options = {};

    ctx.session_proxy->callMethod("remove")
        .onInterface(dnfdaemon::INTERFACE_RPM)
        .withTimeout(static_cast<uint64_t>(-1))
        .withArguments(pkg_specs, options);

    run_transaction(offline_option.get_value());
}

}  // namespace dnfdaemon::client
