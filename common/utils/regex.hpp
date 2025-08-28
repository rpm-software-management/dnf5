// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_UTILS_REGEX_HPP
#define LIBDNF5_UTILS_REGEX_HPP


// Limit the string length, because GCC std::regex_match() exhausts a stack on very long strings.
#define MAX_STRING_LENGTH_FOR_REGEX_MATCH 2 << 10


#endif  // LIBDNF5_UTILS_REGEX_HPP
