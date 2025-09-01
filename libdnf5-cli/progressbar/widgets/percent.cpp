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


#include "libdnf5-cli/progressbar/widgets/percent.hpp"

#include "libdnf5-cli/progressbar/progress_bar.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>


namespace libdnf5::cli::progressbar {


std::size_t PercentWidget::get_total_width() const noexcept {
    return 4 + get_delimiter_before().size();
}


std::string PercentWidget::to_string() const {
    if (!get_visible()) {
        return "";
    }

    std::ostringstream ss;
    ss << get_delimiter_before();
    if (get_bar()->get_percent_done() >= 0) {
        ss << std::right << std::setw(3) << get_bar()->get_percent_done();
    } else {
        ss << std::right << std::setw(3) << "???";
    }
    ss << "%";
    return ss.str();
}


}  // namespace libdnf5::cli::progressbar
