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


#ifndef LIBDNF5_CLI_PROGRESSBAR_MULTI_PROGRESS_BAR_HPP
#define LIBDNF5_CLI_PROGRESSBAR_MULTI_PROGRESS_BAR_HPP


#include "download_progress_bar.hpp"
#include "progress_bar.hpp"

#include "libdnf5-cli/defs.h"

#include "libdnf5/common/impl_ptr.hpp"

#include <iostream>
#include <memory>
#include <vector>


namespace libdnf5::cli::progressbar {


class LIBDNF_CLI_API MultiProgressBar {
public:
    static constexpr std::size_t NEVER_VISIBLE_LIMIT = static_cast<std::size_t>(-1);

    /// Controls which unfinished bars are rendered on each print.
    enum class PrintMode {
        ALL_BARS,         ///< All unfinished bars are rendered on every print.
        ACTIVE_BARS_ONLY  ///< Only bars marked via mark_bar_active() are rendered.
    };

    /// Constructs a MultiProgressBar with the given print mode.
    explicit MultiProgressBar(PrintMode print_mode);

    /// Constructs a MultiProgressBar with PrintMode::ALL_BARS.
    explicit MultiProgressBar();

    ~MultiProgressBar();

    MultiProgressBar(const MultiProgressBar & src) = delete;
    MultiProgressBar & operator=(const MultiProgressBar & src) = delete;

    MultiProgressBar(MultiProgressBar && src) noexcept = delete;
    MultiProgressBar & operator=(MultiProgressBar && src) noexcept = delete;

    PrintMode get_print_mode() const noexcept;

    void add_bar(std::unique_ptr<ProgressBar> && bar);

    /// Marks a bar as active for rendering in the next print.
    /// In modes other than PrintMode::ACTIVE_BARS_ONLY this is a no-op.
    /// Can be called multiple times on the same bar without side effects.
    /// Must not be called on a bar that has already been rendered in a finished
    /// state; doing so would cause it to be counted again as a finished bar
    /// in the next render.
    void mark_bar_active(ProgressBar * bar);

    // In interactive mode MultiProgressBar doesn't print a newline after unfinished progressbars.
    // Finished progressbars always end with a newline.
    void print();

    LIBDNF_CLI_API friend std::ostream & operator<<(std::ostream & stream, MultiProgressBar & mbar);

    /// Returns the total bar.
    DownloadProgressBar & get_total_bar() noexcept;

    /// Sets the minimum number of registered progress bars to show the total bar.
    void set_total_bar_visible_limit(std::size_t value) noexcept;

    /// Sets the visibility of number widget in the total bar.
    void set_total_bar_number_widget_visible(bool value) noexcept;

    /// Allows one to preset the value of the total number of progress bars.
    /// If the value is lower than the current number of registered progress bars, it is automatically increased.
    void set_total_num_of_bars(std::size_t value) noexcept;

    /// Returns the total number of progress bars.
    /// It can be greater than the current number of registered progress bars.
    std::size_t get_total_num_of_bars() const noexcept;

private:
    class LIBDNF_CLI_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};


}  // namespace libdnf5::cli::progressbar


#endif  // LIBDNF5_CLI_PROGRESSBAR_MULTI_PROGRESS_BAR_HPP
