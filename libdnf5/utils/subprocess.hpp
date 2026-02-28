// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_UTILS_SUBPROCESS_HPP
#define LIBDNF5_UTILS_SUBPROCESS_HPP

#include <string>
#include <vector>

namespace libdnf5::utils {

struct CompletedProcess {
    int returncode;
    std::vector<std::byte> stdout;
    std::vector<std::byte> stderr;
};

/// @brief Run a command in a child process and wait for it to complete.
///
/// @param command Command to run. See `man 3 execvp`.
/// @param args Arguments to pass. argv[0] must be the first element of `args`.
/// @returns stdout, stderr, and return code. A negative return code `-N` indicates that the child was terminated by signal `N`.
/// @throws libdnf5::SystemError if any unexpected error occurred while forking, creating pipes, etc. No error is thrown if the command exits with nonzero code.
/// @throws libdnf5::RuntimeError if the child process terminated without a return code or signal (unreachable?)
CompletedProcess run(std::string command, std::vector<std::string> args);

}  // namespace libdnf5::utils

#endif  // LIBDNF5_UTILS_SUBPROCESS_HPP
