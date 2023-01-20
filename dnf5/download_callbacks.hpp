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

#ifndef DNF5_DOWNLOAD_CALLBACKS_HPP
#define DNF5_DOWNLOAD_CALLBACKS_HPP

#include <libdnf-cli/progressbar/multi_progress_bar.hpp>
#include <libdnf/repo/download_callbacks.hpp>

#include <chrono>

namespace dnf5 {

class DownloadCallbacks : public libdnf::repo::DownloadCallbacks {
public:
    void * add_new_download(void * user_data, const char * description, double total_to_download) override;

    int progress(void * user_cb_data, double total_to_download, double downloaded) override;

    int end(void * user_cb_data, TransferStatus status, const char * msg) override;

    int mirror_failure(void * user_cb_data, const char * msg, const char * url) override;

    void reset_progress_bar();

private:
    bool is_time_to_print();
    void print();

    std::unique_ptr<libdnf::cli::progressbar::MultiProgressBar> multi_progress_bar;
    std::chrono::time_point<std::chrono::steady_clock> prev_print_time{std::chrono::steady_clock::now()};
    bool printed{false};
};

}  // namespace dnf5

#endif
