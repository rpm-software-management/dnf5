/*
Copyright (C) 2022 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/dnf5/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "config_automatic.hpp"

#include <limits.h>

#include <cstdlib>

namespace dnf5 {

void ConfigAutomatic::load_from_parser(
    const libdnf5::ConfigParser & parser,
    const libdnf5::Vars & vars,
    libdnf5::Logger & logger,
    libdnf5::Option::Priority priority) {
    config_commands.load_from_parser(parser, "commands", vars, logger, priority);
    config_emitters.load_from_parser(parser, "emitters", vars, logger, priority);
    config_email.load_from_parser(parser, "email", vars, logger, priority);
    config_command.load_from_parser(parser, "command", vars, logger, priority);
    config_command_email.load_from_parser(parser, "command_email", vars, logger, priority);
}


ConfigAutomaticCommands::ConfigAutomaticCommands() {
    opt_binds().add("upgrade_type", upgrade_type);
    opt_binds().add("random_sleep", random_sleep);
    opt_binds().add("network_online_timeout", network_online_timeout);
    opt_binds().add("download_updates", download_updates);
    opt_binds().add("apply_updates", apply_updates);
    opt_binds().add("reboot", reboot);
    opt_binds().add("reboot_command", reboot_command);
}


ConfigAutomaticEmitters::ConfigAutomaticEmitters() {
    opt_binds().add("emit_via", emit_via);
    opt_binds().add("system_name", system_name);
}

std::string ConfigAutomaticEmitters::gethostname() {
    char hostname[HOST_NAME_MAX + 1];
    ::gethostname(hostname, HOST_NAME_MAX + 1);
    return std::string(hostname);
}


ConfigAutomaticEmail::ConfigAutomaticEmail() {
    opt_binds().add("email_to", email_to);
    opt_binds().add("email_from", email_from);
    opt_binds().add("email_host", email_host);
    opt_binds().add("email_port", email_port);
    opt_binds().add("email_tls", email_tls);
}


ConfigAutomaticCommand::ConfigAutomaticCommand() {
    opt_binds().add("command_format", command_format);
    opt_binds().add("stdin_format", stdin_format);
}


ConfigAutomaticCommandEmail::ConfigAutomaticCommandEmail() {
    opt_binds().add("command_format", command_format);
    opt_binds().add("stdin_format", stdin_format);
    opt_binds().add("email_to", email_to);
    opt_binds().add("email_from", email_from);
}


}  // namespace dnf5
