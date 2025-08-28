// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_CLI_EXIT_CODES_HPP
#define LIBDNF5_CLI_EXIT_CODES_HPP

namespace libdnf5::cli {

enum class ExitCode : int { SUCCESS = 0, ERROR = 1, ARGPARSER_ERROR = 2 };

}  // namespace libdnf5::cli

#endif  // LIBDNF5_CLI_EXIT_CODES_HPP
