/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_COMPS_POOL_UTILS_HPP
#define LIBDNF_COMPS_POOL_UTILS_HPP

#include "solv/pool.hpp"

#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace libdnf::comps {

// Search solvables that correspond to the environment_ids for given key
// Return first non-empty string
template <typename T>
std::string lookup_str(libdnf::solv::Pool & pool, std::vector<T> ids, Id key) {
    for (T id : ids) {
        auto value = pool.lookup_str(id.id, key);
        if (value) {
            return value;
        }
    }
    return "";
}


template <typename T>
std::string get_translated_str(libdnf::solv::Pool & pool, std::vector<T> ids, Id key, const char * lang = nullptr) {
    // Go through all environment solvables and return first translation found.
    for (T id : ids) {
        Solvable * solvable = pool.id2solvable(id.id);
        const char * translation = nullptr;
        if (lang) {
            translation = solvable_lookup_str_lang(solvable, key, lang, 1);
        } else {
            translation = solvable_lookup_str_poollang(solvable, key);
        }
        if (translation) {
            // Return translation only if it's different from the untranslated string
            // (solvable_lookup_str_lang returns the untranslated string if there is no translation).
            const char * untranslated = solvable_lookup_str(solvable, key);
            if (translation != untranslated && strcmp(translation, untranslated) != 0) {
                return std::string(translation);
            }
        }
    }
    // If no translation was found, return the untranslated string.
    return lookup_str(pool, ids, key);
}


std::pair<std::string, std::string> split_solvable_name(std::string_view solvable_name);

}  // namespace libdnf::comps

#endif  // LIBDNF_COMPS_POOL_UTILS_HPP
