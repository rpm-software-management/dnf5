// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "libdnf5-cli/progressbar/widgets/time.hpp"

#include "common.hpp"

#include "libdnf5-cli/progressbar/progress_bar.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>


namespace libdnf5::cli::progressbar {


std::size_t TimeWidget::get_total_width() const noexcept {
    return 7 + get_delimiter_before().size();
}


std::string TimeWidget::to_string() const {
    if (!get_visible()) {
        return "";
    }
    std::ostringstream ss;
    ss << get_delimiter_before();
    if (get_bar()->is_finished() || get_bar()->get_total_ticks() < 0) {
        // finished or unknown total ticks -> display elapsed time
        ss << format_time(get_bar()->get_elapsed_seconds(), false);
    } else {
        // in progress -> display remaining time
        ss << format_time(get_bar()->get_remaining_seconds(), false);
    }
    return ss.str();
}


}  // namespace libdnf5::cli::progressbar
