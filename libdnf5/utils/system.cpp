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
#include <libdnf5/rpm/arch.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <rpm/rpmlib.h>

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

std::string get_os() {
    const char * value;
    init_lib_rpm(rpm::detect_arch().c_str());
    rpmGetOsInfo(&value, nullptr);
    return value;
}

}  // namespace libdnf5::utils
