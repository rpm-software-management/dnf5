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


namespace libdnf5::cli::progressbar {

class MultiProgressBar::Impl {
public:
    Impl();

    std::size_t total_bar_visible_limit{0};
    std::vector<std::unique_ptr<ProgressBar>> bars_all;
    std::vector<ProgressBar *> bars_todo;
    std::vector<ProgressBar *> bars_done;
    DownloadProgressBar total;
    // Whether the last line was printed without a new line ending (such as an in progress bar)
    bool line_printed{false};
    std::size_t num_of_lines_to_clear{0};
};

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
    p_impl->bars_todo.push_back(bar.get());

    // if the number is not set, automatically find and set the next available
    if (bar->get_number() == 0) {
        int number = 0;
        for (auto i : p_impl->bars_todo) {
            number = std::max(number, i->get_number());
        }
        bar->set_number(number + 1);
    }

    p_impl->bars_all.push_back(std::move(bar));

    // update total (in [num/total]) in total progress bar
    auto registered_bars_count = static_cast<int32_t>(p_impl->bars_all.size());
    if (p_impl->total.get_total() < registered_bars_count) {
        p_impl->total.set_total(registered_bars_count);
    }

    // update total (in [num/total]) in all bars to do
    for (auto & i : p_impl->bars_todo) {
        i->set_total(p_impl->total.get_total());
    }
}


void MultiProgressBar::set_total_num_of_bars(std::size_t value) noexcept {
    if (value < p_impl->bars_all.size()) {
        value = p_impl->bars_all.size();
    }
    auto num_of_bars = static_cast<int>(value);
    if (num_of_bars != p_impl->total.get_total()) {
        p_impl->total.set_total(num_of_bars);

        // update total (in [num/total]) in all bars to do
        for (auto & i : p_impl->bars_todo) {
            i->set_total(p_impl->total.get_total());
        }
    }
}


std::size_t MultiProgressBar::get_total_num_of_bars() const noexcept {
    return static_cast<std::size_t>(p_impl->total.get_total());
}


std::ostream & operator<<(std::ostream & stream, MultiProgressBar & mbar) {
    const bool is_interactive{tty::is_interactive()};
    auto terminal_width = static_cast<std::size_t>(tty::get_width());

    // We'll buffer the output text to a single string and print it all at once.
    // This is to avoid multiple writes to the terminal, which can cause flickering.
    static std::ostringstream text_buffer;
    text_buffer.str("");
    text_buffer.clear();

    std::size_t last_num_of_lines_to_clear = mbar.p_impl->num_of_lines_to_clear;
    std::size_t num_of_lines_permanent = 0;

    if (is_interactive && mbar.p_impl->num_of_lines_to_clear > 0) {
        if (mbar.p_impl->num_of_lines_to_clear > 1) {
            // Move the cursor up by the number of lines we want to write over
            text_buffer << "\033[" << (mbar.p_impl->num_of_lines_to_clear - 1) << "A";
        }
        text_buffer << "\r";
    }

    // Number of lines that need to be cleared, including the last line even if it is empty
    mbar.p_impl->num_of_lines_to_clear = 0;
    mbar.p_impl->line_printed = false;

    // store numbers of bars in progress
    std::vector<int32_t> numbers;
    for (auto * bar : mbar.p_impl->bars_todo) {
        numbers.insert(numbers.begin(), bar->get_number());
    }

    // print completed bars first and remove them from the list
    for (std::size_t i = 0; i < mbar.p_impl->bars_todo.size(); i++) {
        auto * bar = mbar.p_impl->bars_todo[i];
        if (!bar->is_finished()) {
            continue;
        }
        bar->set_number(numbers.back());
        numbers.pop_back();
        text_buffer << *bar;
        text_buffer << std::endl;
        num_of_lines_permanent++;
        num_of_lines_permanent += bar->calculate_messages_terminal_lines(terminal_width);
        mbar.p_impl->bars_done.push_back(bar);
        // TODO(dmach): use iterator
        mbar.p_impl->bars_todo.erase(mbar.p_impl->bars_todo.begin() + static_cast<int>(i));
        i--;
    }

    // then print incomplete
    for (auto & bar : mbar.p_impl->bars_todo) {
        bar->set_number(numbers.back());
        numbers.pop_back();

        // skip printing bars that haven't started yet
        if (bar->get_state() != libdnf5::cli::progressbar::ProgressBarState::STARTED) {
            bar->update();
            continue;
        }

        if (!is_interactive) {
            bar->update();
            continue;
        }
        if (mbar.p_impl->line_printed) {
            text_buffer << std::endl;
        }
        text_buffer << *bar;
        mbar.p_impl->line_printed = true;
        mbar.p_impl->num_of_lines_to_clear++;
        mbar.p_impl->num_of_lines_to_clear += bar->calculate_messages_terminal_lines(terminal_width);
    }

    // then print the "total" progress bar
    int32_t total_numbers = 0;
    int64_t ticks = 0;
    int64_t total_ticks = 0;

    for (auto & bar : mbar.p_impl->bars_done) {
        total_numbers = std::max(total_numbers, bar->get_total());
        // completed bars can be unfinished
        // add only processed ticks to both values
        total_ticks += bar->get_ticks();
        ticks += bar->get_ticks();
    }

    for (auto & bar : mbar.p_impl->bars_todo) {
        total_numbers = std::max(total_numbers, bar->get_total());
        total_ticks += bar->get_total_ticks();
        ticks += bar->get_ticks();
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
        mbar_total.set_number(static_cast<int>(mbar.p_impl->bars_done.size()));

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

    // If we have written less lines than last time we need to clear the rest otherwise
    // there would be garbage under the updated progressbar. This is because normally
    // we don't actually clear the lines we just write over the old output to ensure smooth
    // output updating.
    // TODO(amatej): It would be sufficient to do this only once after all progressbars have
    // finished but unfortunaly MultiProgressBar doesn't have a pImpl so we cannot
    // store the highest line count it had. We could fix this when breaking ABI.
    size_t all_written = num_of_lines_permanent + mbar.p_impl->num_of_lines_to_clear;
    if (last_num_of_lines_to_clear > all_written) {
        auto delta = last_num_of_lines_to_clear - all_written;
        if (!mbar.p_impl->line_printed) {
            text_buffer << tty::clear_line;
        }
        for (std::size_t i = 0; i < delta; i++) {
            text_buffer << tty::cursor_down << tty::clear_line;
        }
        // Move cursor back up after clearing lines leftover from previous print
        text_buffer << "\033[" << delta << "A";
    }

    stream << text_buffer.str();  // Single syscall to output all commands

    return stream;
}


}  // namespace libdnf5::cli::progressbar
