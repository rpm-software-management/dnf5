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


#include "libdnf5/rpm/arch.hpp"

#include "arch_private.hpp"

#include <algorithm>
#include <cstring>

namespace libdnf5::rpm {

std::vector<std::string> get_supported_arches() {
    std::vector<std::string> arches;
    for (int i = 0; ARCH_MAP[i].base; ++i) {
        for (int j = 0; ARCH_MAP[i].native[j]; ++j) {
            arches.emplace_back(ARCH_MAP[i].native[j]);
        }
    }
    std::sort(arches.begin(), arches.end());
    return arches;
}

std::string get_base_arch(const std::string & arch) {
    auto arch_c = arch.c_str();
    for (int i = 0; libdnf5::rpm::ARCH_MAP[i].base; ++i) {
        for (int j = 0; libdnf5::rpm::ARCH_MAP[i].native[j]; ++j) {
            if (std::strcmp(libdnf5::rpm::ARCH_MAP[i].native[j], arch_c) == 0) {
                return libdnf5::rpm::ARCH_MAP[i].base;
            }
        }
    }
    return {};
}

}  // namespace libdnf5::rpm
