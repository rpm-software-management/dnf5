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


#ifndef LIBDNF_CLI_PROGRESSBAR_MULTI_PROGRESS_BAR_HPP
#define LIBDNF_CLI_PROGRESSBAR_MULTI_PROGRESS_BAR_HPP


#include "download_progress_bar.hpp"
#include "progress_bar.hpp"

#include <iostream>
#include <memory>
#include <vector>


namespace libdnf::cli::progressbar {


class MultiProgressBar {
public:
    explicit MultiProgressBar();
    ~MultiProgressBar();

    void add_bar(std::unique_ptr<ProgressBar> && bar);
    void print() {
        std::cout << *this;
        std::cout << std::flush;
    }
    friend std::ostream & operator<<(std::ostream & stream, MultiProgressBar & mbar);

private:
    std::vector<std::unique_ptr<ProgressBar>> bars_all;
    std::vector<ProgressBar *> bars_todo;
    std::vector<ProgressBar *> bars_done;
    DownloadProgressBar total;
    std::size_t printed_lines = 0;
};


}  // namespace libdnf::cli::progressbar


#endif  // LIBDNF_CLI_PROGRESSBAR_MULTI_PROGRESS_BAR_HPP
