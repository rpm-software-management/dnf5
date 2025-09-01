// Copyright (C) 2021 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

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
