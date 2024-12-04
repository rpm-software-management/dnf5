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


#ifndef LIBDNF5_CLI_PROGRESSBAR_MULTI_PROGRESS_BAR_HPP
#define LIBDNF5_CLI_PROGRESSBAR_MULTI_PROGRESS_BAR_HPP


#include "download_progress_bar.hpp"
#include "progress_bar.hpp"

#include "libdnf5-cli/defs.h"

#include <iostream>
#include <memory>
#include <vector>


namespace libdnf5::cli::progressbar {


class LIBDNF_CLI_API MultiProgressBar {
public:
    static constexpr std::size_t NEVER_VISIBLE_LIMIT = static_cast<std::size_t>(-1);

    explicit MultiProgressBar();
    ~MultiProgressBar();

    void add_bar(std::unique_ptr<ProgressBar> && bar);

    // In interactive mode MultiProgressBar doesn't print a newline after unfinished progressbars.
    // Finished progressbars always end with a newline.
    void print() {
        std::cerr << *this;
        std::cerr << std::flush;
    }
    LIBDNF_CLI_API friend std::ostream & operator<<(std::ostream & stream, MultiProgressBar & mbar);

    /// Sets the minimum number of registered progress bars to show the total bar.
    void set_total_bar_visible_limit(std::size_t value) noexcept { total_bar_visible_limit = value; }

    /// Sets the visibility of number widget in the total bar.
    void set_total_bar_number_widget_visible(bool value) noexcept { total.set_number_widget_visible(value); }

    /// Allows to preset the value of the total number of progress bars.
    /// If the value is lower than the current number of registered progress bars, it is automatically increased.
    void set_total_num_of_bars(std::size_t value) noexcept;

    /// Returns the total number of progress bars.
    /// It can be greater than the current number of registered progress bars.
    std::size_t get_total_num_of_bars() const noexcept;

private:
    std::size_t total_bar_visible_limit{0};
    std::vector<std::unique_ptr<ProgressBar>> bars_all;
    std::vector<ProgressBar *> bars_todo;
    std::vector<ProgressBar *> bars_done;
    DownloadProgressBar total;
    // Whether the last line was printed without a new line ending (such as an in progress bar)
    bool line_printed{false};
    std::size_t num_of_lines_to_clear{0};
};


}  // namespace libdnf5::cli::progressbar


#endif  // LIBDNF5_CLI_PROGRESSBAR_MULTI_PROGRESS_BAR_HPP
