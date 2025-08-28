// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "libdnf5-cli/progressbar/widgets/speed.hpp"

#include "common.hpp"

#include "libdnf5-cli/progressbar/progress_bar.hpp"


namespace libdnf5::cli::progressbar {


std::size_t SpeedWidget::get_total_width() const noexcept {
    return 11 + get_delimiter_before().size();
}


std::string SpeedWidget::to_string() const {
    if (!get_visible()) {
        return "";
    }
    if (get_bar()->is_finished() || get_bar()->get_total_ticks() < 0) {
        // finished -> display average speed
        return get_delimiter_before() + format_size(get_bar()->get_average_speed()) + "/s";
    } else {
        // in progress -> display current speed
        return get_delimiter_before() + format_size(get_bar()->get_current_speed()) + "/s";
    }
}


}  // namespace libdnf5::cli::progressbar
