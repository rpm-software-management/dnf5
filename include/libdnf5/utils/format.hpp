// Copyright (C) 2021 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef LIBDNF5_UTILS_FORMAT_HPP
#define LIBDNF5_UTILS_FORMAT_HPP

#include <fmt/format.h>


namespace libdnf5::utils {

/// Format `args` according to the `runtime_format_string`, and return the result as a string.
/// Unlike C++20 `std::format`, the format string of the `libdnf5::sformat` function template
/// does not have to be a constant expression.
template <typename... Args>
inline std::string sformat(std::string_view runtime_format_string, Args &&... args) {
    return fmt::vformat(runtime_format_string, fmt::make_format_args(args...));
}

}  // namespace libdnf5::utils

#endif  // LIBDNF5_UTILS_FORMAT_HPP
