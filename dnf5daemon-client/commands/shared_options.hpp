// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

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
