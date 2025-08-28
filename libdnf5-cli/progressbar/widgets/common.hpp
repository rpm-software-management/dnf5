// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_PROGRESSBAR_WIDGETS_COMMON_HPP
#define LIBDNF5_CLI_PROGRESSBAR_WIDGETS_COMMON_HPP


#include <cstdint>
#include <string>


namespace libdnf5::cli::progressbar {


std::string format_size(int64_t num);
std::string format_time(int64_t num, bool negative);


}  // namespace libdnf5::cli::progressbar


#endif  // LIBDNF5_CLI_PROGRESSBAR_WIDGETS_COMMON_HPP
