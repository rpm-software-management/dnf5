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

#ifndef LIBDNF5_UTILS_LOCALE_HPP
#define LIBDNF5_UTILS_LOCALE_HPP

#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/defs.h"

#include <locale.h>

#include <locale>


namespace libdnf5::utils {

/// Class for passing locale
class LIBDNF_API Locale {
public:
    /// Creates a new `Locale` object containing the system locale with the specified `std_name` (for example, "C",
    /// or "POSIX", or "cs_CZ.UTF-8") if such a locale is supported by the operating system.
    ///
    /// @param std_name name of the system locale to use ("" - user-preferred locale, must not be null pointer)
    explicit Locale(const char * std_name);

    ~Locale();

    const std::locale & get_cpp_locale() const;
    ::locale_t get_c_locale() const;

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5::utils

#endif  // LIBDNF5_UTILS_LOCALE_HPP
