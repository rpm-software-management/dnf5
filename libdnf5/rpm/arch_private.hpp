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

#ifndef LIBDNF5_RPM_ARCH_PRIVATE_HPP
#define LIBDNF5_RPM_ARCH_PRIVATE_HPP

namespace libdnf5::rpm {

#define MAX_NATIVE_ARCHES 12

// data taken from DNF
// TODO(mblaha): think about using a C++ structure for this e.g. std::map<std::string_view>
static const struct {
    const char * base;
    const char * native[MAX_NATIVE_ARCHES];
} ARCH_MAP[] = {
    {"aarch64", {"aarch64", nullptr}},
    {"alpha",
     {"alpha",
      "alphaev4",
      "alphaev45",
      "alphaev5",
      "alphaev56",
      "alphaev6",
      "alphaev67",
      "alphaev68",
      "alphaev7",
      "alphapca56",
      nullptr}},
    {"arm", {"armv5tejl", "armv5tel", "armv5tl", "armv6l", "armv7l", "armv8l", nullptr}},
    {"armhfp", {"armv6hl", "armv7hl", "armv7hnl", "armv8hl", "armv8hnl", "armv8hcnl", nullptr}},
    {"i386", {"i386", "athlon", "geode", "i486", "i586", "i686", nullptr}},
    {"ia64", {"ia64", nullptr}},
    {"mips", {"mips", nullptr}},
    {"mipsel", {"mipsel", nullptr}},
    {"mips64", {"mips64", nullptr}},
    {"mips64el", {"mips64el", nullptr}},
    {"noarch", {"noarch", nullptr}},
    {"ppc", {"ppc", nullptr}},
    {"ppc64", {"ppc64", "ppc64iseries", "ppc64p7", "ppc64pseries", nullptr}},
    {"ppc64le", {"ppc64le", nullptr}},
    {"riscv32", {"riscv32", nullptr}},
    {"riscv64", {"riscv64", nullptr}},
    {"riscv128", {"riscv128", nullptr}},
    {"s390", {"s390", nullptr}},
    {"s390x", {"s390x", nullptr}},
    {"sh3", {"sh3", nullptr}},
    {"sh4", {"sh4", "sh4a", nullptr}},
    {"sparc", {"sparc", "sparc64", "sparc64v", "sparcv8", "sparcv9", "sparcv9v", nullptr}},
    {"x86_64", {"x86_64", "amd64", "ia32e", nullptr}},
    {"loongarch64", {"loongarch64", nullptr}},
    {nullptr, {nullptr}}};

}  // namespace libdnf5::rpm

#endif  // LIBDNF5_RPM_ARCH_PRIVATE_HPP
