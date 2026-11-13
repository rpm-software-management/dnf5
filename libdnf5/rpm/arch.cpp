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


#include "libdnf5/rpm/arch.hpp"

#include "arch_private.hpp"

#include "libdnf5/common/exception.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <sys/auxv.h>
#include <sys/utsname.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>

namespace libdnf5::rpm {

/* ARM specific HWCAP defines may be missing on non-ARM devices */
#ifndef HWCAP_ARM_VFP
#define HWCAP_ARM_VFP (1 << 6)
#endif
#ifndef HWCAP_ARM_NEON
#define HWCAP_ARM_NEON (1 << 12)
#endif

std::string detect_arch() {
    struct utsname un;

    if (uname(&un) < 0) {
        throw RuntimeError(M_("Failed to execute uname()"));
    }

    if (!strncmp(un.machine, "armv", 4)) {
        /* un.machine is armvXE, where X is version number and E is
         * endianness (b or l); we need to add modifiers such as
         * h (hardfloat), n (neon). Neon is a requirement of armv8 so
         * as far as rpm is concerned armv8l is the equivalent of armv7hnl
         * (or 7hnb) so we don't explicitly add 'n' for 8+ as it's expected. */
        char endian = un.machine[strlen(un.machine) - 1];
        char * modifier = un.machine + 5;
        while (isdigit(*modifier)) /* keep armv7, armv8, armv9, armv10, armv100, ... */
            modifier++;
        if (getauxval(AT_HWCAP) & HWCAP_ARM_VFP)
            *modifier++ = 'h';
        if ((atoi(un.machine + 4) == 7) && (getauxval(AT_HWCAP) & HWCAP_ARM_NEON))
            *modifier++ = 'n';
        *modifier++ = endian;
        *modifier = 0;
    }
#ifdef __MIPSEL__
    // support for little endian MIPS
    if (!strcmp(un.machine, "mips"))
        strcpy(un.machine, "mipsel");
    else if (!strcmp(un.machine, "mips64"))
        strcpy(un.machine, "mips64el");
#endif
    return un.machine;
}

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
