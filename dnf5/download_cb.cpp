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

#include "download_cb.hpp"

#include <libdnf-cli/progressbar/download_progress_bar.hpp>
#include <libdnf-cli/progressbar/multi_progress_bar.hpp>

#include <chrono>
#include <memory>

std::chrono::time_point<std::chrono::steady_clock> DownloadCB::prev_print_time = std::chrono::steady_clock::now();

DownloadCB::DownloadCB(libdnf::cli::progressbar::MultiProgressBar & mp_bar, const std::string & what)
    : multi_progress_bar(&mp_bar),
      what(what) {
    auto pb = std::make_unique<libdnf::cli::progressbar::DownloadProgressBar>(-1, what);
    progress_bar = pb.get();
    multi_progress_bar->add_bar(std::move(pb));
}


int DownloadCB::end(TransferStatus status, const char * msg) {
    switch (status) {
        case TransferStatus::SUCCESSFUL:
            progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
            break;
        case TransferStatus::ALREADYEXISTS:
            // skipping the download -> downloading 0 bytes
            progress_bar->set_ticks(0);
            progress_bar->set_total_ticks(0);
            progress_bar->add_message(libdnf::cli::progressbar::MessageType::SUCCESS, msg);
            progress_bar->start();
            progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::SUCCESS);
            break;
        case TransferStatus::ERROR:
            progress_bar->add_message(libdnf::cli::progressbar::MessageType::ERROR, msg);
            progress_bar->set_state(libdnf::cli::progressbar::ProgressBarState::ERROR);
            break;
    }
    multi_progress_bar->print();
    return 0;
}

int DownloadCB::progress(double total_to_download, double downloaded) {
    auto total = static_cast<int64_t>(total_to_download);
    if (total > 0) {
        progress_bar->set_total_ticks(total);
    }
    if (progress_bar->get_state() == libdnf::cli::progressbar::ProgressBarState::READY) {
        progress_bar->start();
    }
    progress_bar->set_ticks(static_cast<int64_t>(downloaded));
    if (is_time_to_print()) {
        multi_progress_bar->print();
    }
    return 0;
}

int DownloadCB::mirror_failure(const char * msg, const char * url) {
    std::string message = std::string(msg) + " - " + url;
    progress_bar->add_message(libdnf::cli::progressbar::MessageType::ERROR, message);
    multi_progress_bar->print();
    return 0;
}

bool DownloadCB::is_time_to_print() {
    auto now = std::chrono::steady_clock::now();
    auto delta = now - prev_print_time;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
    if (ms > 100) {
        // 100ms equals to 10 FPS and that seems to be smooth enough
        prev_print_time = now;
        return true;
    }
    return false;
}
