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


#include "libdnf5-cli/progressbar/widgets/number.hpp"

#include "common.hpp"

#include "libdnf5-cli/progressbar/progress_bar.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>


namespace libdnf5::cli::progressbar {


std::size_t NumberWidget::get_total_width() const noexcept {
    return 3 + (2 * get_number_width()) + get_delimiter_before().size();
}


std::size_t NumberWidget::get_number_width() const {
    std::size_t result = std::max(
        static_cast<std::size_t>((get_bar()->get_number() > 0) ? log10(get_bar()->get_number()) : 0),
        static_cast<std::size_t>((get_bar()->get_total() > 0) ? log10(get_bar()->get_total()) : 0));
    return result + 1;
}


std::string NumberWidget::to_string() const {
    if (!get_visible()) {
        return "";
    }
    std::ostringstream ss;
    auto number_width = get_number_width();
    ss << get_delimiter_before();
    ss << "[";
    ss << std::setw(static_cast<int32_t>(number_width)) << get_bar()->get_number();
    ss << "/";
    ss << std::setw(static_cast<int32_t>(number_width)) << get_bar()->get_total();
    ss << "]";
    return ss.str();
}


}  // namespace libdnf5::cli::progressbar
