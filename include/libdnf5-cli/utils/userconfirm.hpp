// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBDNF5_CLI_UTILS_USERCONFIRM_HPP
#define LIBDNF5_CLI_UTILS_USERCONFIRM_HPP

#include "libdnf5-cli/defs.h"

#include <langinfo.h>
#include <libdnf5/conf/config_main.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <regex.h>

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

    auto default_yes = config.get_defaultyes_option().get_value();

    const char * msg_id;
    const char * msg_translated;

    if (default_yes) {
        msg_id = "Is this ok [Y/n]: ";
        msg_translated = dgettext("libdnf5-cli", "Is this ok [Y/n]: ");
    } else {
        msg_id = "Is this ok [y/N]: ";
        msg_translated = dgettext("libdnf5-cli", "Is this ok [y/N]: ");
    }

    // If the prompt was translated, we assume the system locale (nl_langinfo) matches the language of the prompt.
    // If the prompt fell back to English (no translation found), we MUST use English regexes to avoid
    // dangerous mismatches (e.g. Swahili system locale where 'n' means Yes, but displaying English [y/N]).
    std::string yes_pattern;
    std::string no_pattern;

    if (std::string(msg_translated) != msg_id) {
        yes_pattern = nl_langinfo(YESEXPR);
        no_pattern = nl_langinfo(NOEXPR);
    } else {
        yes_pattern = "^[yY]";
        no_pattern = "^[nN]";
    }

    while (true) {
        std::cerr << msg_translated;

        std::string choice;
        std::getline(std::cin, choice);

        if (choice.empty()) {
            return default_yes;
        }

        // Match user input against translator-provided patterns
        // These patterns stay in sync with the translated prompt
        regex_t regex;
        if (regcomp(&regex, yes_pattern.c_str(), REG_EXTENDED | REG_NOSUB) == 0) {
            int result = regexec(&regex, choice.c_str(), 0, nullptr, 0);
            regfree(&regex);
            if (result == 0) {
                return true;
            }
        }

        if (regcomp(&regex, no_pattern.c_str(), REG_EXTENDED | REG_NOSUB) == 0) {
            int result = regexec(&regex, choice.c_str(), 0, nullptr, 0);
            regfree(&regex);
            if (result == 0) {
                return false;
            }
        }

        // Fallback: accept English y/n if they didn't match the localized patterns
        if (choice == "y" || choice == "Y") {
            return true;
        }
        if (choice == "n" || choice == "N") {
            return false;
        }

        // If patterns didn't match, the input is invalid - loop and ask again
    }
}

};  // namespace libdnf5::cli::utils::userconfirm

#endif  // LIBDNF5_CLI_UTILS_USERCONFIRM_HPP
