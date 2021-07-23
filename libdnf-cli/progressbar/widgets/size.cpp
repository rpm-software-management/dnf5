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


#include "common.hpp"

#include "libdnf-cli/progressbar/progress_bar.hpp"
#include "libdnf-cli/progressbar/widgets/size.hpp"


namespace libdnf::cli::progressbar {


std::size_t SizeWidget::get_total_width() const noexcept {
    return 9 + get_delimiter_before().size();
}


std::string SizeWidget::to_string() const {
    if (!get_visible()) {
        return "";
    }
    return get_delimiter_before() + format_size(get_bar()->get_ticks());
}


}  // namespace libdnf::cli::progressbar
