/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "utils.hpp"

#include <unistd.h>

#include <iostream>

namespace dnfdaemon::client {

bool am_i_root() noexcept {
    return geteuid() == 0;
}

// TODO(mblaha): enhance (see dnf4's userconfirm implementation), move to libdnf-cli
bool userconfirm(Context & ctx) {
    // "assumeno" takes precedence over "assumeyes"
    if (ctx.assume_no.get_value()) {
        return false;
    }
    if (ctx.assume_yes.get_value()) {
        return true;
    }
    std::string msg = "Is this ok [y/N]: ";
    while (true) {
        std::cout << msg;

        std::string choice;
        std::getline(std::cin, choice);

        if (choice.empty()) {
            return false;
        }
        if (choice == "y" || choice == "Y") {
            return true;
        }
        if (choice == "n" || choice == "N") {
            return false;
        }
    }
}

}  // namespace dnfdaemon::client
