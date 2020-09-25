/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef TEST_LIBDNF_UTILS_HPP
#define TEST_LIBDNF_UTILS_HPP

#include "libdnf/rpm/package_set.hpp"

#include <cppunit/extensions/HelperMacros.h>

#include <iterator>
#include <vector>


/// Implements assertion on std collections (e.g. asserting two vectors are equal).
/// https://stackoverflow.com/questions/43709391/cppunit-assert-equal-on-std-collections
namespace CPPUNIT_NS {
    template <template <typename...> class C, typename... T>
    struct assertion_traits<C<T...>> {
        inline static bool equal(const C<T...> & left, const C<T...> & right) {
            if (left.size() != right.size()) {
                return false;
            }
            return std::equal(
                left.cbegin(), left.cend(), right.cbegin(), right.cend(),
                assertion_traits<decltype(*(left.cbegin()))>::equal
            );
        }

        inline static std::string toString(const C<T...> & x) {
            std::ostringstream os;
            os << "[";
            std::transform(
                x.cbegin(), x.cend(), std::ostream_iterator<std::string>(os, ", "),
                assertion_traits<decltype(*(x.cbegin()))>::toString);
            os << "]";
            return os.str();
        }

        assertion_traits<C<T...>>() = delete;
        ~assertion_traits<C<T...>>() = delete;
        assertion_traits<C<T...>>(const assertion_traits<C<T...>>&) = delete;
        assertion_traits<C<T...>>& operator=(const assertion_traits<C<T...>>&) = delete;
    };
}


/// Converts ReldepList to a vector of strings for easy assertions.
std::vector<std::string> to_vector(const libdnf::rpm::ReldepList & rdl);


#endif  // TEST_LIBDNF_UTILS_HPP
