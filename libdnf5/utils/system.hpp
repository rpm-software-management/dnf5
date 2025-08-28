// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef SYSTEM_HPP
#define SYSTEM_HPP
#include <string>

namespace libdnf5::utils {

void init_lib_rpm(const char * arch);
std::string detect_arch();
std::string get_os();

}  // namespace libdnf5::utils

#endif  // SYSTEM_HPP
