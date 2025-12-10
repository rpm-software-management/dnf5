// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_CLI_UTILS_JSON_HPP
#define LIBDNF5_CLI_UTILS_JSON_HPP

#include "libdnf5-cli/defs.h"

#include <cctype>
#include <string>

namespace libdnf5::cli::utils::json {

/// Normalizes the field name for a JSON field, i.e., converts to lower-case and
/// replaces all whitespace with underscores.
LIBDNF_CLI_API std::string normalize_field(std::string name) {
    for (char & c : name) {
        if (std::isspace(c)) {
            c = '_';
        } else {
            c = static_cast<char>(std::tolower(c));
        }
    }
    return name;
}

};  // namespace libdnf5::cli::utils::json

#endif  // LIBDNF5_CLI_UTILS_JSON_HPP
