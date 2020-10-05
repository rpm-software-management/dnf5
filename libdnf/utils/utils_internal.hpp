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


#ifndef LIBDNF_UTILS_UTILS_INTERNAL_HPP
#define LIBDNF_UTILS_UTILS_INTERNAL_HPP


extern "C" {
#include <solv/pool.h>
}

#include <ctype.h>
#include <cstring>


namespace libdnf::utils {


inline bool is_glob_pattern(const char * pattern) {
    return strpbrk(pattern, "*[?") != nullptr;
}

/// @brief Test if pattern is file path
/// Return true if pattern start with "/" or pattern[0] == '*' && pattern[1] == '/'
static inline bool is_file_pattern(const std::string & pattern) {
    return pattern[0] == '/' || (pattern[0] == '*' && pattern[1] == '/');
}

inline Id id_to_lowercase_id(Pool * pool, const char * name_cstring, int create) {
    int name_length = static_cast<int>(strlen(name_cstring));
    auto tmp_name_cstring = pool_alloctmpspace(pool, name_length);
    for (int index = 0; index < name_length; ++index) {
        tmp_name_cstring[index] = static_cast<char>(tolower(name_cstring[index]));
    }
    return pool_strn2id(pool, tmp_name_cstring, static_cast<unsigned int>(name_length), create);
}

inline Id id_to_lowercase_id(Pool * pool, Id id_input, int create) {
    auto name_cstring = pool_id2str(pool, id_input);
    return id_to_lowercase_id(pool, name_cstring, create);
}


}  // namespace libdnf::utils


#endif  // LIBDNF_UTILS_UTILS_INTERNAL_HPP
