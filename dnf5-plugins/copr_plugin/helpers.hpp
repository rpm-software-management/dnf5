// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DNF5_COMMANDS_COPR_HELPERS_HPP
#define DNF5_COMMANDS_COPR_HELPERS_HPP

#include <libdnf5/utils/format.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace dnf5 {

std::vector<std::string> repo_fallbacks(const std::string & name_version);
template <typename... Args>
void warning(const char * format, Args &&... args) {
    std::cerr << "WARNING: " + libdnf5::utils::sformat(format, std::forward<Args>(args)...) << std::endl;
}

}  // namespace dnf5

#endif  // DNF5_COMMANDS_COPR_HELPERS_HPP
