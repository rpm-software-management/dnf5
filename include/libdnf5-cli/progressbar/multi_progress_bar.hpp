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

    /// Controls when bar state changes are processed for rendering.
    enum class TrackingMode {
        ON_RENDER,  ///< All bars are scanned on every render to detect state changes.
        ON_CHANGE   ///< State changes are tracked incrementally via bar_*() wrapper methods.
                    ///< Much faster when many bars are registered but only a few are active.
    };

    /// Constructs a MultiProgressBar with the given tracking mode.
    explicit MultiProgressBar(TrackingMode tracking_mode);

    /// Constructs a MultiProgressBar with TrackingMode::ON_RENDER.
    explicit MultiProgressBar();

    ~MultiProgressBar();

    MultiProgressBar(const MultiProgressBar & src) = delete;
    MultiProgressBar & operator=(const MultiProgressBar & src) = delete;

    MultiProgressBar(MultiProgressBar && src) noexcept = delete;
    MultiProgressBar & operator=(MultiProgressBar && src) noexcept = delete;

    /// Returns the tracking mode set at construction.
    TrackingMode get_tracking_mode() const noexcept;

    void add_bar(std::unique_ptr<ProgressBar> && bar);

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

    /// @name Methods to work with registered bars
    /// Preferred way to modify a bar registered in this MultiProgressBar.
    /// Use these instead of calling ProgressBar methods directly, so that
    /// MultiProgressBar can track state changes and use them for rendering
    /// optimizations. Calls on a finished bar are ignored for mutating methods.
    /// @{

    /// Returns the number of processed ticks.
    int64_t bar_get_ticks(ProgressBar & bar) const noexcept;
    /// Sets the number of processed ticks.
    void bar_set_ticks(ProgressBar & bar, int64_t value);
    /// Adds to the number of processed ticks.
    void bar_add_ticks(ProgressBar & bar, int64_t value);

    /// Returns the total number of ticks.
    int64_t bar_get_total_ticks(ProgressBar & bar) const noexcept;
    /// Sets the total number of ticks.
    void bar_set_total_ticks(ProgressBar & bar, int64_t value);

    /// Returns the display number of the bar.
    int32_t bar_get_number(ProgressBar & bar) const noexcept;

    /// Starts the bar (transitions from READY to STARTED).
    void bar_start(ProgressBar & bar);
    /// Returns the current state.
    ProgressBarState bar_get_state(ProgressBar & bar) const noexcept;
    /// Sets the state. Setting back to READY is not allowed.
    void bar_set_state(ProgressBar & bar, ProgressBarState value);
    /// Returns true if the bar has reached a terminal state (SUCCESS or ERROR).
    bool bar_is_finished(ProgressBar & bar) const noexcept;
    /// Returns true if the bar is in the ERROR state.
    bool bar_is_failed(ProgressBar & bar) const noexcept;

    /// Returns the description string.
    std::string bar_get_description(ProgressBar & bar) const noexcept;
    /// Sets the description string.
    void bar_set_description(ProgressBar & bar, const std::string & value);

    /// Adds a message to the bar.
    void bar_add_message(ProgressBar & bar, MessageType type, const std::string & message);
    /// Removes the last message.
    void bar_pop_message(ProgressBar & bar);
    /// Returns all messages.
    const std::vector<ProgressBar::Message> & bar_get_messages(ProgressBar & bar) const noexcept;
    /// Returns the message prefix string.
    const std::string & bar_get_message_prefix(ProgressBar & bar) const noexcept;

    /// Returns whether auto-finish is enabled.
    bool bar_get_auto_finish(ProgressBar & bar) const noexcept;
    /// Enables or disables auto-finish. Disable to manage the terminal state manually.
    void bar_set_auto_finish(ProgressBar & bar, bool value);

    /// Returns the percentage of ticks done (0–100).
    int32_t bar_get_percent_done(ProgressBar & bar) const noexcept;
    /// Returns the current (recent) speed in ticks per second.
    int64_t bar_get_current_speed(ProgressBar & bar) const noexcept;
    /// Returns the average speed in ticks per second.
    int64_t bar_get_average_speed(ProgressBar & bar) const noexcept;
    /// Returns elapsed time in seconds.
    int64_t bar_get_elapsed_seconds(ProgressBar & bar) const noexcept;
    /// Returns estimated remaining time in seconds.
    int64_t bar_get_remaining_seconds(ProgressBar & bar) const noexcept;

    /// @}

private:
    class LIBDNF_CLI_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};


}  // namespace libdnf5::cli::progressbar


#endif  // LIBDNF5_CLI_PROGRESSBAR_MULTI_PROGRESS_BAR_HPP
