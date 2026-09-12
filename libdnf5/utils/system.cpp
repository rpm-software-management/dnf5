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

/* Taken from RPM: https://github.com/rpm-software-management/rpm/blob/bb93973956444e4530dd8439f4b9002c03128070/lib/rpmrc.cc#L671 */
#if defined(__linux__) && defined(__x86_64__)
#if defined(__has_builtin)
#if __has_builtin(__builtin_cpu_supports)
#define HAS_BUILTIN_CPU_SUPPORTS
#endif
#endif
#if defined(HAS_BUILTIN_CPU_SUPPORTS)
static inline void cpuid(uint32_t op, uint32_t op2, uint32_t * eax, uint32_t * ebx, uint32_t * ecx, uint32_t * edx) {
    asm volatile("cpuid\n" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(op), "c"(op2));
}

/* From gcc's gcc/config/i386/cpuid.h */
/* Features (%eax == 1) */
/* %ecx */
#define bit_SSE3       (1 << 0)
#define bit_SSSE3      (1 << 9)
#define bit_FMA        (1 << 12)
#define bit_CMPXCHG16B (1 << 13)
#define bit_SSE4_1     (1 << 19)
#define bit_SSE4_2     (1 << 20)
#define bit_MOVBE      (1 << 22)
#define bit_POPCNT     (1 << 23)
#define bit_OSXSAVE    (1 << 27)
#define bit_AVX        (1 << 28)
#define bit_F16C       (1 << 29)

/* Extended Features (%eax == 0x80000001) */
/* %ecx */
#define bit_LAHF_LM (1 << 0)
#define bit_LZCNT   (1 << 5)

/* Extended Features (%eax == 7) */
/* %ebx */
#define bit_BMI      (1 << 3)
#define bit_AVX2     (1 << 5)
#define bit_BMI2     (1 << 8)
#define bit_AVX512F  (1 << 16)
#define bit_AVX512DQ (1 << 17)
#define bit_AVX512CD (1 << 28)
#define bit_AVX512BW (1 << 30)
#define bit_AVX512VL (1u << 31)

static int get_x86_64_level(void) {
    int level = 1;

    unsigned int op_0_eax = 0, op_1_ecx = 0, op_80000001_ecx = 0, op_7_ebx = 0, unused;
    cpuid(0, 0, &op_0_eax, &unused, &unused, &unused);
    cpuid(1, 0, &unused, &unused, &op_1_ecx, &unused);
    cpuid(0x80000001, 0, &unused, &unused, &op_80000001_ecx, &unused);
    cpuid(7, 0, &unused, &op_7_ebx, &unused, &unused);

    /* CPUID is unfortunately not quite enough: It indicates whether the hardware supports it,
     * but software support (kernel/hypervisor) is also needed for register saving/restoring.
     * For that we can ask the compiler's runtime lib through __builtin_cpu_supports. This is
     * not usable as a complete replacement for the CPUID code though as not all extensions are
     * supported (yet). */

    const unsigned int op_1_ecx_lv2 = bit_SSE3 | bit_SSSE3 | bit_CMPXCHG16B | bit_SSE4_1 | bit_SSE4_2 | bit_POPCNT;
    if ((op_1_ecx & op_1_ecx_lv2) == op_1_ecx_lv2 && (op_80000001_ecx & bit_LAHF_LM))
        level = 2;

    const unsigned int op_1_ecx_lv3 = bit_FMA | bit_MOVBE | bit_OSXSAVE | bit_AVX | bit_F16C;
    const unsigned int op_7_ebx_lv3 = bit_BMI | bit_AVX2 | bit_BMI2;
    if (level == 2 && (op_1_ecx & op_1_ecx_lv3) == op_1_ecx_lv3 && op_0_eax >= 7 &&
        (op_7_ebx & op_7_ebx_lv3) == op_7_ebx_lv3 && (op_80000001_ecx & bit_LZCNT) && __builtin_cpu_supports("avx2")) {
        level = 3;
    }

    const unsigned int op_7_ebx_lv4 = bit_AVX512F | bit_AVX512DQ | bit_AVX512CD | bit_AVX512BW | bit_AVX512VL;
    if (level == 3 && (op_7_ebx & op_7_ebx_lv4) == op_7_ebx_lv4 && __builtin_cpu_supports("avx512f")) {
        level = 4;
    }

    return level;
}
#else /* defined(HAS_BUILTIN_CPU_SUPPORTS) */
static int get_x86_64_level(void) {
    return 1;
}
#endif
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
#if defined(__linux__) && defined(__x86_64__)
    {
        int x86_64_level = get_x86_64_level();
        if (x86_64_level > 1) {
            strcpy(un.machine, "x86_64_vX");
            un.machine[8] = '0' + x86_64_level;
        }
    }
#endif
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
