// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DNF5DAEMON_CLIENT_COMMANDS_SHARED_OPTIONS_HPP
#define DNF5DAEMON_CLIENT_COMMANDS_SHARED_OPTIONS_HPP

#include <libdnf5-cli/argument_parser.hpp>

namespace dnfdaemon::client {

libdnf5::cli::ArgumentParser::PositionalArg * pkg_specs_argument(
    libdnf5::cli::ArgumentParser & parser, int nargs, std::vector<std::string> & pkg_specs);

/// Create the `--offline` named arg bound to the given option
libdnf5::cli::ArgumentParser::NamedArg * create_offline_option(
    libdnf5::cli::ArgumentParser & parser, libdnf5::Option * value);

}  // namespace dnfdaemon::client
#endif
