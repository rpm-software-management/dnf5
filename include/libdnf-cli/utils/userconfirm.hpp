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

#ifndef LIBDNF_CLI_UTILS_USERCONFIRM_HPP
#define LIBDNF_CLI_UTILS_USERCONFIRM_HPP

#include "libdnf/conf/config_main.hpp"

#include <iostream>
#include <string>

namespace libdnf::cli::utils::userconfirm {

/// Asks the user for confirmation. The default answer is taken from the configuration.

template <class Config>
bool userconfirm(Config & config) {
    for (const auto & tsflag : config.tsflags().get_value()) {
        if (tsflag == "test") {
            std::cout << "Test mode enabled: Only package downloads, gpg key installations and transaction checks will "
                         "be performed."
                      << std::endl;
            break;
        }
    }

    // "assumeno" takes precedence over "assumeyes"
    if (config.assumeno().get_value()) {
        return false;
    }
    if (config.assumeyes().get_value()) {
        return true;
    }
    std::string msg;
    if (config.defaultyes().get_value()) {
        msg = "Is this ok [Y/n]: ";
    } else {
        msg = "Is this ok [y/N]: ";
    }
    while (true) {
        std::cout << msg;

        std::string choice;
        std::getline(std::cin, choice);

        if (choice.empty()) {
            return config.defaultyes().get_value();
        }
        if (choice == "y" || choice == "Y") {
            return true;
        }
        if (choice == "n" || choice == "N") {
            return false;
        }
    }
}

};  // namespace libdnf::cli::utils::userconfirm

#endif  // LIBDNF_CLI_UTILS_USERCONFIRM_HPP
