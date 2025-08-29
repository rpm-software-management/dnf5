// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_PRIVATE_ACCESSOR_HPP
#define TEST_LIBDNF5_PRIVATE_ACCESSOR_HPP


// A getter generating structure
#define create_private_getter_template                 \
    template <typename AccessTag, typename T, T value> \
    struct PrivateGetter {                             \
        friend constexpr auto get(AccessTag) {         \
            return value;                              \
        }                                              \
    }

// Helper macro with automatic member type deduction
#define create_getter(AccessTag, class_member_ptr) \
    struct AccessTag {};                           \
    constexpr auto get(AccessTag);                 \
    template struct PrivateGetter<AccessTag, decltype(class_member_ptr), class_member_ptr>

// Helper macro with member type as argument. Needed in case of overloaded members.
#define create_getter_type(AccessTag, class_member_type, class_member_ptr) \
    struct AccessTag {};                                                   \
    constexpr auto get(AccessTag);                                         \
    template struct PrivateGetter<AccessTag, class_member_type, class_member_ptr>


#endif  // TEST_LIBDNF5_PRIVATE_ACCESSOR_HPP
