// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_PROGRESSBAR_WIDGETS_SPEED_HPP
#define LIBDNF5_CLI_PROGRESSBAR_WIDGETS_SPEED_HPP


#include "widget.hpp"


namespace libdnf5::cli::progressbar {


class SpeedWidget : public Widget {
public:
    std::size_t get_total_width() const noexcept override;
    std::string to_string() const override;
};


}  // namespace libdnf5::cli::progressbar


#endif  // LIBDNF5_CLI_PROGRESSBAR_WIDGETS_SPEED_HPP
