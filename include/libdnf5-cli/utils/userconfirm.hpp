// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_CLI_UTILS_USERCONFIRM_HPP
#define LIBDNF5_CLI_UTILS_USERCONFIRM_HPP

#include <libdnf5/conf/config_main.hpp>

#include <iostream>
#include <string>

namespace libdnf5::cli::utils::userconfirm {

/// Asks the user for confirmation. The default answer is taken from the configuration.

template <class Config>
bool userconfirm(Config & config) {
    // "assumeno" takes precedence over "assumeyes"
    if (config.get_assumeno_option().get_value()) {
        return false;
    }
    if (config.get_assumeyes_option().get_value()) {
        return true;
    }
    std::string msg;
    if (config.get_defaultyes_option().get_value()) {
        msg = "Is this ok [Y/n]: ";
    } else {
        msg = "Is this ok [y/N]: ";
    }
    while (true) {
        std::cerr << msg;

        std::string choice;
        std::getline(std::cin, choice);

        if (choice.empty()) {
            return config.get_defaultyes_option().get_value();
        }
        if (choice == "y" || choice == "Y") {
            return true;
        }
        if (choice == "n" || choice == "N") {
            return false;
        }
    }
}

};  // namespace libdnf5::cli::utils::userconfirm

#endif  // LIBDNF5_CLI_UTILS_USERCONFIRM_HPP
