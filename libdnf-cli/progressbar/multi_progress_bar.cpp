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


#include "libdnf-cli/progressbar/multi_progress_bar.hpp"

#include "libdnf-cli/tty.hpp"

#include <unistd.h>

#include <algorithm>
#include <iomanip>


namespace libdnf::cli::progressbar {


MultiProgressBar::MultiProgressBar() : total(0, "Total") {
    total.set_auto_finish(false);
    total.start();
    if (tty::is_interactive()) {
        std::cout << tty::cursor_hide;
    }
}


MultiProgressBar::~MultiProgressBar() {
    if (tty::is_interactive()) {
        std::cout << tty::cursor_show;
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
    total.set_total(static_cast<int>(bars_all.size()));

    // update total (in [num/total]) in all bars
    for (auto & i : bars_all) {
        i->set_total(total.get_total());
    }
}


std::ostream & operator<<(std::ostream & stream, MultiProgressBar & mbar) {
    if (tty::is_interactive()) {
        stream << tty::clear_line;
        for (std::size_t i = 1; i < mbar.printed_lines; i++) {
            stream << tty::cursor_up << tty::clear_line;
        }
        stream << "\r";
    }
    mbar.printed_lines = 0;

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
        stream << *bar;
        stream << std::endl;
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
        if (bar->get_state() != libdnf::cli::progressbar::ProgressBarState::STARTED) {
            bar->update();
            continue;
        }

        if (!tty::is_interactive()) {
            bar->update();
            continue;
        }
        stream << *bar;
        stream << std::endl;
        mbar.printed_lines++;
        mbar.printed_lines += bar->get_messages().size();
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


    if (tty::is_interactive() || mbar.bars_todo.empty()) {
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
        mbar.printed_lines += 2;
    }

    return stream;
}


}  // namespace libdnf::cli::progressbar
