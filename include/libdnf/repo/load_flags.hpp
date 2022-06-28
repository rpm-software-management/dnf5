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

#ifndef LIBDNF_REPO_LOAD_FLAGS_HPP
#define LIBDNF_REPO_LOAD_FLAGS_HPP


#include <type_traits>


namespace libdnf::repo {


// TODO(lukash) name this RepodataType?
enum class LoadFlags {
    PRIMARY = 1 << 1,     /// Load primary repodata (primary.xml).
    FILELISTS = 1 << 2,   /// Load file lists (filelists.xml). Requires loading PRIMARY.
    OTHER = 1 << 3,       /// Load changelogs (other.xml). Requires loading PRIMARY.
    PRESTO = 1 << 4,      /// Use deltarpm.
    UPDATEINFO = 1 << 5,  /// Load advisories (updateinfo.xml).
    COMPS = 1 << 6,       /// Load comps groups and environments (comps.xml).
    ALL = ~0,             /// Load all possible repodata.
};


inline constexpr LoadFlags operator|(LoadFlags lhs, LoadFlags rhs) {
    return static_cast<LoadFlags>(
        static_cast<std::underlying_type_t<LoadFlags>>(lhs) | static_cast<std::underlying_type_t<LoadFlags>>(rhs));
}


inline constexpr LoadFlags operator&(LoadFlags lhs, LoadFlags rhs) {
    return static_cast<LoadFlags>(
        static_cast<std::underlying_type_t<LoadFlags>>(lhs) & static_cast<std::underlying_type_t<LoadFlags>>(rhs));
}


inline constexpr bool any(LoadFlags flags) {
    return static_cast<typename std::underlying_type<LoadFlags>::type>(flags) != 0;
}


}  // namespace libdnf::repo


#endif  // LIBDNF_REPO_LOAD_FLAGS_HPP
