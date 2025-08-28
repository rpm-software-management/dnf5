// Copyright Contributors to the DNF5 project.
// Copyright (C) 2022 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef LIBDNF5_UTILS_TO_UNDERLYING_HPP
#define LIBDNF5_UTILS_TO_UNDERLYING_HPP

#include <fmt/format.h>

#include <type_traits>

namespace libdnf5::utils {

/// Converts an enumeration to its underlying type.
/// `std::to_underlying` is planned for C++23.
template <class Enum>
constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept {
    return static_cast<std::underlying_type_t<Enum>>(e);
}

}  // namespace libdnf5::utils

#endif  // LIBDNF5_UTILS_TO_UNDERLYING_HPP
