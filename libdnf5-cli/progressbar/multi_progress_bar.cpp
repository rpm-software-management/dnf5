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


#include "libdnf5-cli/progressbar/multi_progress_bar.hpp"

#include "libdnf5-cli/tty.hpp"

#include "libdnf5/utils/bgettext/bgettext-lib.h"

#include <unistd.h>

#include <algorithm>
#include <iomanip>
#include <limits>
#include <utility>


namespace libdnf5::cli::progressbar {

class MultiProgressBar::Impl {
public:
    Impl();

    std::size_t total_bar_visible_limit{0};
    std::vector<std::unique_ptr<ProgressBar>> bars_all;
    std::vector<ProgressBar *> bars_todo;
    std::vector<ProgressBar *> bars_done;
    DownloadProgressBar total;
    int64_t done_ticks{0};
    // Whether the last line was printed without a new line ending (such as an in progress bar)
    bool line_printed{false};
    std::size_t num_of_lines_to_clear{0};
};


namespace {

using BarNumberType = decltype(std::declval<ProgressBar>().get_number());

/// Safely casts any numeric value to BarNumberType, clamping it within bounds.
BarNumberType cast_to_bar_number(auto value) {
    // Check if the input value exceeds the maximum limit of the target type
    if (std::cmp_greater(value, std::numeric_limits<BarNumberType>::max())) {
        return std::numeric_limits<BarNumberType>::max();
    }

    // Check if the input value is below the minimum limit of the target type
    if (std::cmp_less(value, std::numeric_limits<BarNumberType>::min())) {
        return std::numeric_limits<BarNumberType>::min();
    }

    return static_cast<BarNumberType>(value);
}

}  // namespace


MultiProgressBar::Impl::Impl() : total(0, _("Total")) {
    total.set_auto_finish(false);
    total.start();
}


MultiProgressBar::MultiProgressBar() : p_impl(new Impl()) {
    if (tty::is_interactive()) {
        std::cerr << tty::cursor_hide;
    }
}

MultiProgressBar::~MultiProgressBar() {
    if (tty::is_interactive()) {
        std::cerr << tty::cursor_show;
    }
}

void MultiProgressBar::print() {
    std::cerr << *this;
    std::cerr << std::flush;
}

void MultiProgressBar::set_total_bar_visible_limit(std::size_t value) noexcept {
    p_impl->total_bar_visible_limit = value;
}

DownloadProgressBar & MultiProgressBar::get_total_bar() noexcept {
    return p_impl->total;
}

void MultiProgressBar::set_total_bar_number_widget_visible(bool value) noexcept {
    p_impl->total.set_number_widget_visible(value);
}

void MultiProgressBar::add_bar(std::unique_ptr<ProgressBar> && bar) {
    // set bar number, clamp to bounds on overflow
    auto next_number = cast_to_bar_number(p_impl->bars_all.size() + 1);
    bar->set_number(next_number);

    // register bar to MultiProgressBar
    p_impl->bars_todo.push_back(bar.get());
    p_impl->bars_all.push_back(std::move(bar));

    // update total (in [num/total]) in total progress bar
    auto registered_bars_count = next_number;
    if (p_impl->total.get_total() < registered_bars_count) {
        p_impl->total.set_total(registered_bars_count);
    }
}


void MultiProgressBar::set_total_num_of_bars(std::size_t value) noexcept {
    if (value < p_impl->bars_all.size()) {
        value = p_impl->bars_all.size();
    }
    auto num_of_bars = cast_to_bar_number(value);
    if (num_of_bars != p_impl->total.get_total()) {
        p_impl->total.set_total(num_of_bars);
    }
}


std::size_t MultiProgressBar::get_total_num_of_bars() const noexcept {
    return static_cast<std::size_t>(p_impl->total.get_total());
}


std::ostream & operator<<(std::ostream & stream, MultiProgressBar & mbar) {
    const bool is_interactive{tty::is_interactive()};
    auto terminal_width = static_cast<std::size_t>(tty::get_width());
    auto total_num_of_bars = mbar.p_impl->total.get_total();

    // We'll buffer the output text to a single string and print it all at once.
    // This is to avoid multiple writes to the terminal, which can cause flickering.
    static std::ostringstream text_buffer;
    text_buffer.str("");
    text_buffer.clear();

    if (is_interactive && mbar.p_impl->num_of_lines_to_clear > 0) {
        if (mbar.p_impl->num_of_lines_to_clear > 1) {
            // Move the cursor up by the number of lines we want to write over
            text_buffer << "\033[" << (mbar.p_impl->num_of_lines_to_clear - 1) << "A";
        }
        text_buffer << "\r";
        text_buffer << tty::clear_to_end;
    }

    // Number of lines that need to be cleared, including the last line even if it is empty
    mbar.p_impl->num_of_lines_to_clear = 0;
    mbar.p_impl->line_printed = false;

    // initialize bar number counter to bars_done.size(), clamp to bounds on overflow
    auto number = cast_to_bar_number(mbar.p_impl->bars_done.size());

    // print completed bars first and move them from bars_todo to bars_done
    for (auto it = mbar.p_impl->bars_todo.begin(); it != mbar.p_impl->bars_todo.end();) {
        auto * const bar = *it;

        if (!bar->is_finished()) {
            ++it;
            continue;
        }

        if (number < std::numeric_limits<decltype(number)>::max()) {
            ++number;
        }
        bar->set_number(number);
        bar->set_total(total_num_of_bars);
        text_buffer << *bar;
        text_buffer << std::endl;

        if (const auto bar_ticks = bar->get_ticks(); bar_ticks > 0) {
            mbar.p_impl->done_ticks += bar_ticks;
        }
        mbar.p_impl->bars_done.push_back(bar);
        it = mbar.p_impl->bars_todo.erase(it);
    }

    // then print incomplete
    for (auto & bar : mbar.p_impl->bars_todo) {
        if (number < std::numeric_limits<decltype(number)>::max()) {
            ++number;
        }
        bar->set_number(number);

        // skip printing bars that haven't started yet
        if (bar->get_state() != libdnf5::cli::progressbar::ProgressBarState::STARTED) {
            continue;
        }

        if (!is_interactive) {
            bar->update();
            continue;
        }
        if (mbar.p_impl->line_printed) {
            text_buffer << std::endl;
        }
        bar->set_total(total_num_of_bars);
        text_buffer << *bar;
        mbar.p_impl->line_printed = true;
        mbar.p_impl->num_of_lines_to_clear++;
        mbar.p_impl->num_of_lines_to_clear += bar->calculate_messages_terminal_lines(terminal_width);
    }

    // then print the "total" progress bar
    // completed bars can be unfinished
    // add only processed ticks to both values
    int64_t ticks = mbar.p_impl->done_ticks;
    int64_t total_ticks = ticks;

    for (auto & bar : mbar.p_impl->bars_todo) {
        if (const auto bar_total_ticks = bar->get_total_ticks(); bar_total_ticks > 0) {
            total_ticks += bar_total_ticks;
        }
        if (const auto bar_ticks = bar->get_ticks(); bar_ticks > 0) {
            ticks += bar_ticks;
        }
    }

    if ((mbar.p_impl->bars_all.size() >= mbar.p_impl->total_bar_visible_limit) &&
        (is_interactive || mbar.p_impl->bars_todo.empty())) {
        if (mbar.p_impl->line_printed) {
            text_buffer << std::endl;
        }
        // print divider
        text_buffer << std::string(terminal_width, '-');
        text_buffer << std::endl;

        // print Total progress bar
        auto & mbar_total = mbar.get_total_bar();
        mbar_total.set_number(cast_to_bar_number(mbar.p_impl->bars_done.size()));

        mbar_total.set_total_ticks(total_ticks);
        mbar_total.set_ticks(ticks);

        if (mbar.p_impl->bars_todo.empty()) {
            // all bars have finished, set the "Total" bar as finished too according to their states
            mbar_total.set_state(ProgressBarState::SUCCESS);
        }

        text_buffer << mbar_total;
        // +1 for the divider line, +1 for the total bar line
        mbar.p_impl->num_of_lines_to_clear += 2;
        if (mbar_total.is_finished()) {
            text_buffer << std::endl;
        } else {
            mbar.p_impl->line_printed = true;
        }
    }

    stream << text_buffer.str();  // Single syscall to output all commands

    return stream;
}


}  // namespace libdnf5::cli::progressbar
