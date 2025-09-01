// Copyright Contributors to the DNF5 project.
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


#ifndef LIBDNF5_CLI_PROGRESSBAR_WIDGETS_WIDGET_HPP
#define LIBDNF5_CLI_PROGRESSBAR_WIDGETS_WIDGET_HPP


#include <string>


namespace libdnf5::cli::progressbar {


class ProgressBar;


class Widget {
public:
    bool get_visible() const noexcept { return visible; }
    void set_visible(bool value) noexcept { visible = value; };

    ProgressBar * get_bar() const noexcept { return bar; }
    void set_bar(ProgressBar * value) { bar = value; }

    virtual std::size_t get_total_width() const noexcept = 0;
    virtual std::string to_string() const = 0;

    const std::string & get_delimiter_before() const noexcept { return delimiter_before; }
    void set_delimiter_before(const std::string & value) { delimiter_before = value; }

private:
    ProgressBar * bar = nullptr;
    bool visible{true};
    std::string delimiter_before;
};


}  // namespace libdnf5::cli::progressbar


#endif  // LIBDNF5_CLI_PROGRESSBAR_WIDGETS_WIDGET_HPP
