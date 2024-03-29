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


#ifndef LIBDNF5_CLI_PROGRESSBAR_WIDGETS_DESCRIPTION_HPP
#define LIBDNF5_CLI_PROGRESSBAR_WIDGETS_DESCRIPTION_HPP


#include "widget.hpp"


namespace libdnf5::cli::progressbar {


class DescriptionWidget : public Widget {
public:
    std::size_t get_total_width() const noexcept override;

    void set_total_width(std::size_t value);
    void set_width(std::size_t value) { width = value; }

    std::string to_string() const override;

private:
    std::size_t width = 0;
};


}  // namespace libdnf5::cli::progressbar


#endif  // LIBDNF5_CLI_PROGRESSBAR_WIDGETS_DESCRIPTION_HPP
