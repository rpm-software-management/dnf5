// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


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
