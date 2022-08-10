/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef TEST_LIBDNF_PRIVATE_ACCESSOR_HPP
#define TEST_LIBDNF_PRIVATE_ACCESSOR_HPP


// A getter generating structure
template <typename AccessTag, auto value>
struct CreateGetter {
    friend constexpr auto get(AccessTag) { return value; }
};

// Helper macro
#define create_getter(AccessTag, class_member_ptr) \
    struct AccessTag {};                           \
    constexpr auto get(AccessTag);                 \
    template struct CreateGetter<AccessTag, class_member_ptr>;


#endif  // TEST_LIBDNF_PRIVATE_ACCESSOR_HPP
