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


#include "libdnf5-cli/progressbar/widgets/progress.hpp"

#include "libdnf5-cli/progressbar/progress_bar.hpp"

#include <cmath>
#include <iomanip>
#include <string>


namespace libdnf5::cli::progressbar {


std::size_t ProgressWidget::get_total_width() const noexcept {
    return get_width() + get_delimiter_before().size();
}


std::string ProgressWidget::to_string() const {
    if (!get_visible()) {
        return "";
    }
    std::ostringstream os;
    std::size_t progress = 0;

    os << get_delimiter_before();
    os << "[";
    if (get_bar()->get_total_ticks() > 0 && get_bar()->get_ticks() > 0) {
        progress = static_cast<std::size_t>(rint(
            static_cast<double>(get_bar()->get_ticks()) / static_cast<double>(get_bar()->get_total_ticks()) *
            (static_cast<double>(width) - 2)));
        os << std::left << std::setw(static_cast<int>(width - 2)) << std::string(progress, '=');
    } else {
        auto now = std::chrono::system_clock::now();
        auto delta = now - get_bar()->get_begin();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
        progress = static_cast<std::size_t>(ms / 500) % (width - 4);
        os << std::right << std::string(progress, ' ') << "<=>" << std::string((width - 5) - progress, ' ');
    }
    os << "]";
    return os.str();
}


}  // namespace libdnf5::cli::progressbar
