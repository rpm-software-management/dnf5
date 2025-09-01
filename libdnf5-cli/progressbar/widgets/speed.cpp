// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


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
