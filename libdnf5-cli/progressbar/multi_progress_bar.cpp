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


#include "libdnf5-cli/progressbar/multi_progress_bar.hpp"

#include "libdnf5-cli/tty.hpp"

#include <unistd.h>

#include <algorithm>
#include <iomanip>


namespace libdnf5::cli::progressbar {


MultiProgressBar::MultiProgressBar() : total(0, "Total") {
    total.set_auto_finish(false);
    total.start();
    if (tty::is_interactive()) {
        std::cerr << tty::cursor_hide;
    }
}


MultiProgressBar::~MultiProgressBar() {
    if (tty::is_interactive()) {
        std::cerr << tty::cursor_show;
    }
}


void MultiProgressBar::add_bar(std::unique_ptr<ProgressBar> && bar) {
    bars_todo.push_back(bar.get());

    // if the number is not set, automatically find and set the next available
    if (bar->get_number() == 0) {
        int number = 0;
        for (auto i : bars_todo) {
            number = std::max(number, i->get_number());
        }
        bar->set_number(number + 1);
    }

    bars_all.push_back(std::move(bar));

    // update total (in [num/total]) in total progress bar
    auto registered_bars_count = static_cast<int32_t>(bars_all.size());
    if (total.get_total() < registered_bars_count) {
        total.set_total(registered_bars_count);
    }

    // update total (in [num/total]) in all bars to do
    for (auto & i : bars_todo) {
        i->set_total(total.get_total());
    }
}


void MultiProgressBar::set_total_num_of_bars(std::size_t value) noexcept {
    if (value < bars_all.size()) {
        value = bars_all.size();
    }
    auto num_of_bars = static_cast<int>(value);
    if (num_of_bars != total.get_total()) {
        total.set_total(num_of_bars);

        // update total (in [num/total]) in all bars to do
        for (auto & i : bars_todo) {
            i->set_total(total.get_total());
        }
    }
}


std::size_t MultiProgressBar::get_total_num_of_bars() const noexcept {
    return static_cast<std::size_t>(total.get_total());
}


std::ostream & operator<<(std::ostream & stream, MultiProgressBar & mbar) {
    const bool is_interactive{tty::is_interactive()};

    if (is_interactive && mbar.num_of_lines_to_clear > 0) {
        stream << tty::clear_line;
        for (std::size_t i = 1; i < mbar.num_of_lines_to_clear; i++) {
            stream << tty::cursor_up << tty::clear_line;
        }
        stream << "\r";
    } else if (mbar.line_printed) {
        stream << std::endl;
    }
    mbar.num_of_lines_to_clear = 0;
    mbar.line_printed = false;

    // store numbers of bars in progress
    std::vector<int32_t> numbers;
    for (auto * bar : mbar.bars_todo) {
        numbers.insert(numbers.begin(), bar->get_number());
    }

    // print completed bars first and remove them from the list
    for (std::size_t i = 0; i < mbar.bars_todo.size(); i++) {
        auto * bar = mbar.bars_todo[i];
        if (!bar->is_finished()) {
            continue;
        }
        bar->set_number(numbers.back());
        numbers.pop_back();
        if (mbar.line_printed) {
            stream << std::endl;
        }
        stream << *bar;
        mbar.line_printed = true;
        mbar.bars_done.push_back(bar);
        // TODO(dmach): use iterator
        mbar.bars_todo.erase(mbar.bars_todo.begin() + static_cast<int>(i));
        i--;
    }

    // then print incomplete
    for (auto & bar : mbar.bars_todo) {
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
        if (mbar.line_printed) {
            stream << std::endl;
        }
        stream << *bar;
        mbar.line_printed = true;
        mbar.num_of_lines_to_clear++;
        mbar.num_of_lines_to_clear += bar->get_messages().size();
    }

    // then print the "total" progress bar
    int32_t total_numbers = 0;
    int64_t ticks = 0;
    int64_t total_ticks = 0;

    for (auto & bar : mbar.bars_done) {
        total_numbers = std::max(total_numbers, bar->get_total());
        // completed bars can be unfinished
        // add only processed ticks to both values
        total_ticks += bar->get_ticks();
        ticks += bar->get_ticks();
    }

    for (auto & bar : mbar.bars_todo) {
        total_numbers = std::max(total_numbers, bar->get_total());
        total_ticks += bar->get_total_ticks();
        ticks += bar->get_ticks();
    }


    if ((mbar.bars_all.size() >= mbar.total_bar_visible_limit) && (is_interactive || mbar.bars_todo.empty())) {
        if (mbar.line_printed) {
            stream << std::endl;
        }
        // print divider
        int terminal_width = tty::get_width();
        stream << std::string(static_cast<std::size_t>(terminal_width), '-');
        stream << std::endl;

        // print Total progress bar
        mbar.total.set_number(static_cast<int>(mbar.bars_done.size()));

        mbar.total.set_total_ticks(total_ticks);
        mbar.total.set_ticks(ticks);

        if (mbar.bars_todo.empty()) {
            // all bars have finished, set the "Total" bar as finished too according to their states
            mbar.total.set_state(ProgressBarState::SUCCESS);
        }

        stream << mbar.total;
        mbar.num_of_lines_to_clear += 2;
    }

    return stream;
}


}  // namespace libdnf5::cli::progressbar
