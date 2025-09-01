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

#ifndef LIBDNF5_COMMON_SACK_QUERY_CMP_HPP
#define LIBDNF5_COMMON_SACK_QUERY_CMP_HPP

#include <cstdint>
#include <type_traits>


namespace libdnf5::sack {

/// Match operators used in filter methods of the Query objects.
//
// This was inspired by Django's lookups: https://docs.djangoproject.com/en/3.0/ref/models/querysets/#field-lookups
// Read and follow their documentation prior making any changes. We want to remain consistent with them.
enum class QueryCmp : uint32_t {
    // MODIFIERS

    /// Negation modifier. Must be used with a match.
    NOT = (1 << 0),

    /// Case-insensitive modifier. Must be used with a match.
    ICASE = (1 << 1),

    // NULL

    /// Value is NULL/None.
    ISNULL = (1 << 7),

    // NUMBERS

    /// Equal. The same as `EXACT`. Applicable to numbers.
    EQ = (1 << 8),

    /// Not equal. The same as `NOT_EXACT`. Applicable to numbers.
    NEQ = (NOT | EQ),

    /// Greater than. Applicable to numbers.
    GT = (1 << 9),

    /// Greater than or equal to. Applicable to numbers.
    GTE = (GT | EQ),

    /// Less than. Applicable to numbers.
    LT = (1 << 10),

    /// Less than or equal to. Applicable to numbers.
    LTE = (LT | EQ),

    // STRINGS

    /// Case-sensitive exact match. The same as `EQ`. Applicable to strings.
    EXACT = (EQ),

    /// Negative case-sensitive exact match. The same as `NEQ`. Applicable to strings.
    NOT_EXACT = (NOT | EQ),

    /// Case-insensitive exact match. Applicable to strings.
    IEXACT = (ICASE | EXACT),

    /// Negative case-insensitive exact match. Applicable to strings.
    NOT_IEXACT = (NOT | IEXACT),

    /// Case-sensitive containment match. Applicable to strings.
    CONTAINS = (1 << 16),

    /// Negative case-sensitive containment match. Applicable to strings.
    NOT_CONTAINS = (NOT | CONTAINS),

    /// Case-insensitive containment match. Applicable to strings.
    ICONTAINS = (ICASE | CONTAINS),

    /// Negative case-insensitive containment match. Applicable to strings.
    NOT_ICONTAINS = (NOT | ICONTAINS),

    /// Case-sensitive starts-with match. Applicable to strings.
    STARTSWITH = (1 << 17),

    /// Case-insensitive starts-with match. Applicable to strings.
    ISTARTSWITH = (ICASE | STARTSWITH),

    /// Case-sensitive ends-with match. Applicable to strings.
    ENDSWITH = (1 << 18),

    /// Case-insensitive ends-with match. Applicable to strings.
    IENDSWITH = (ICASE | ENDSWITH),

    /// Case-sensitive regular expression match. Applicable to strings.
    REGEX = (1 << 19),

    /// Case-insensitive regular expression match. Applicable to strings.
    IREGEX = (ICASE | REGEX),

    /// Case-sensitive glob match. Applicable to strings.
    GLOB = (1 << 20),

    /// Negative case-sensitive glob match. Applicable to strings.
    NOT_GLOB = (NOT | GLOB),

    /// Case-insensitive glob match. Applicable to strings.
    IGLOB = (ICASE | GLOB),

    /// Negative case-insensitive glob match. Applicable to strings.
    NOT_IGLOB = (NOT | IGLOB),
};


inline constexpr QueryCmp operator|(QueryCmp lhs, QueryCmp rhs) noexcept {
    return static_cast<QueryCmp>(
        static_cast<std::underlying_type<QueryCmp>::type>(lhs) |
        static_cast<std::underlying_type<QueryCmp>::type>(rhs));
}


inline constexpr QueryCmp operator&(QueryCmp lhs, QueryCmp rhs) noexcept {
    return static_cast<QueryCmp>(
        static_cast<std::underlying_type<QueryCmp>::type>(lhs) &
        static_cast<std::underlying_type<QueryCmp>::type>(rhs));
}


/// Returns the value of the left operand with the bits zeroed that are set in the right operand.
/// Can be used eg for removing `NOT` or `ICASE` flags.
inline constexpr QueryCmp operator-(QueryCmp lhs, QueryCmp rhs) noexcept {
    return static_cast<QueryCmp>(
        static_cast<std::underlying_type<QueryCmp>::type>(lhs) &
        ~static_cast<std::underlying_type<QueryCmp>::type>(rhs));
}

}  // namespace libdnf5::sack

#endif  // LIBDNF5_COMMON_SACK_QUERY_CMP_HPP
