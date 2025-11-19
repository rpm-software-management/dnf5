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

#include "libdnf5-cli/utils/userconfirm.hpp"

#include <libdnf5/utils/bgettext/bgettext-lib.h>

namespace libdnf5::cli::utils::userconfirm {

LIBDNF_CLI_API const char * get_yes_no_prompt(bool default_yes) {
    if (default_yes) {
        return _("Is this ok [Y/n]: ");
    } else {
        return _("Is this ok [y/N]: ");
    }
}

LIBDNF_CLI_API const char * get_yes_response() {
    return _("y");
}
LIBDNF_CLI_API const char * get_yes_response_upper() {
    return _("Y");
}
LIBDNF_CLI_API const char * get_no_response() {
    return _("n");
}
LIBDNF_CLI_API const char * get_no_response_upper() {
    return _("N");
}

}  // namespace libdnf5::cli::utils::userconfirm
