// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "utils.hpp"

#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/format.hpp>

#include <chrono>
#include <iomanip>

std::string format_comment(std::string_view cmd) {
    // format the comment for new config file entries
    auto current_time_point = std::chrono::system_clock::now();
    const std::time_t current_time = std::chrono::system_clock::to_time_t(current_time_point);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&current_time), "%F %T");
    // TODO(mblaha): add full command line
    return libdnf5::utils::sformat(_("Added by 'versionlock {}' command on {}"), cmd, ss.str());
}
