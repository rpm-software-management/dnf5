// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "libdnf5-cli/progressbar/widgets/size.hpp"

#include "common.hpp"

#include "libdnf5-cli/progressbar/progress_bar.hpp"


namespace libdnf5::cli::progressbar {


std::size_t SizeWidget::get_total_width() const noexcept {
    return 9 + get_delimiter_before().size();
}


std::string SizeWidget::to_string() const {
    if (!get_visible()) {
        return "";
    }
    return get_delimiter_before() + format_size(get_bar()->get_ticks());
}


}  // namespace libdnf5::cli::progressbar
