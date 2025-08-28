// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "libdnf5-cli/progressbar/widgets/description.hpp"

#include "common.hpp"

#include "libdnf5-cli/progressbar/progress_bar.hpp"

#include <iomanip>


namespace libdnf5::cli::progressbar {


std::size_t DescriptionWidget::get_total_width() const noexcept {
    if (width > 0) {
        return width + get_delimiter_before().size();
    }
    return get_bar()->get_description().size() + get_delimiter_before().size();
}


void DescriptionWidget::set_total_width(std::size_t value) {
    set_width(value - get_delimiter_before().size());
}


std::string DescriptionWidget::to_string() const {
    if (!get_visible()) {
        return "";
    }
    std::ostringstream ss;
    ss << get_delimiter_before();
    ss << std::left;
    ss << std::setw(static_cast<int>(get_total_width() - get_delimiter_before().size()));
    ss << get_bar()->get_description();
    return ss.str();
}


}  // namespace libdnf5::cli::progressbar
