/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DNF5_DOWNLOAD_CB_HPP
#define DNF5_DOWNLOAD_CB_HPP

#include <libdnf-cli/progressbar/download_progress_bar.hpp>
#include <libdnf-cli/progressbar/multi_progress_bar.hpp>
#include <libdnf/repo/download_callbacks.hpp>

#include <memory>

class DownloadCB : public libdnf::repo::DownloadCallbacks {
public:
    DownloadCB(libdnf::cli::progressbar::MultiProgressBar & mp_bar, const std::string & what);

    int end(TransferStatus status, const char * msg) override;
    int progress(double total_to_download, double downloaded) override;
    int mirror_failure(const char * msg, const char * url) override;

private:
    static bool is_time_to_print();

    static std::chrono::time_point<std::chrono::steady_clock> prev_print_time;

    libdnf::cli::progressbar::MultiProgressBar * multi_progress_bar;
    libdnf::cli::progressbar::DownloadProgressBar * progress_bar;
    std::string what;
};


class DownloadCBFactory : public libdnf::repo::DownloadCallbacksFactory {
public:
    DownloadCBFactory();
    ~DownloadCBFactory();

    std::unique_ptr<libdnf::repo::DownloadCallbacks> create_callbacks(const std::string & url) override;

    std::unique_ptr<libdnf::repo::DownloadCallbacks> create_callbacks(const libdnf::rpm::Package & package) override;

    // true if a new progressbar was added to the multi_progress_bar
    bool progressbar_created{false};

private:
    std::unique_ptr<libdnf::cli::progressbar::MultiProgressBar> multi_progress_bar;
};

#endif  // DNF5_DOWNLOAD_CB_HPP
