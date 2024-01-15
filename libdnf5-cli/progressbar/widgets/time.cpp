/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


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
        ss << format_time(get_bar()->get_remaining_seconds(), true);
    }
    return ss.str();
}


}  // namespace libdnf5::cli::progressbar
