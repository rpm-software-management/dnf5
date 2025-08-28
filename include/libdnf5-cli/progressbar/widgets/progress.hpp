// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_PROGRESSBAR_WIDGETS_PROGRESS_HPP
#define LIBDNF5_CLI_PROGRESSBAR_WIDGETS_PROGRESS_HPP


#include "widget.hpp"


namespace libdnf5::cli::progressbar {


class ProgressWidget : public Widget {
public:
    std::size_t get_total_width() const noexcept override;

    std::size_t get_width() const noexcept { return width; }
    void set_width(std::size_t value) { width = value; }

    std::string to_string() const override;

private:
    std::size_t width = 20;
};


}  // namespace libdnf5::cli::progressbar


#endif  // LIBDNF5_CLI_PROGRESSBAR_WIDGETS_PROGRESS_HPP
