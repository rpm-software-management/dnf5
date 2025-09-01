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

#include "system.hpp"

#include <libdnf5/common/exception.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <rpm/rpmlib.h>
#include <sys/auxv.h>
#include <sys/utsname.h>

namespace libdnf5::utils {

void init_lib_rpm(const char * arch) {
    static bool lib_rpm_initiated{false};
    if (!lib_rpm_initiated) {
        if (rpmReadConfigFiles(nullptr, arch) != 0) {
            throw RuntimeError(M_("failed to read rpm config files"));
        }
        lib_rpm_initiated = true;
    }
}

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

std::string get_os() {
    const char * value;
    init_lib_rpm(detect_arch().c_str());
    rpmGetOsInfo(&value, nullptr);
    return value;
}

}  // namespace libdnf5::utils
