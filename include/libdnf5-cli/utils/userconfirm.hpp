// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_CLI_UTILS_USERCONFIRM_HPP
#define LIBDNF5_CLI_UTILS_USERCONFIRM_HPP

#include "libdnf5-cli/defs.h"

namespace libdnf5::cli::utils::userconfirm {

/// Asks the user for interactive confirmation.
/// Returns false immediately if assume_no is set, true if assume_yes is set.
/// Otherwise prompts the user; default_yes determines the answer when the user
/// presses Enter without typing anything, and the prompt displayed ([Y/n] or [y/N]).
LIBDNF_CLI_API bool userconfirm(bool assume_no, bool assume_yes, bool default_yes);

/// Asks the user for confirmation. Reads assumeyes and assumeno from config;
/// uses default_yes as the default answer when neither is set.
template <class Config>
bool userconfirm(Config & config, bool default_yes) {
    return userconfirm(
        config.get_assumeno_option().get_value(), config.get_assumeyes_option().get_value(), default_yes);
}

/// Asks the user for confirmation. Reads assumeyes, assumeno, and the default
/// answer from the configuration's defaultyes option.
template <class Config>
bool userconfirm(Config & config) {
    return userconfirm(
        config.get_assumeno_option().get_value(),
        config.get_assumeyes_option().get_value(),
        config.get_defaultyes_option().get_value());
}

};  // namespace libdnf5::cli::utils::userconfirm

#endif  // LIBDNF5_CLI_UTILS_USERCONFIRM_HPP
