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

#include "download_callbacks.hpp"

namespace dnf5 {

void * DownloadCallbacks::add_new_download(
    [[maybe_unused]] void * user_data, const char * description, double total_to_download) {
    if (!multi_progress_bar) {
        multi_progress_bar = std::make_unique<libdnf::cli::progressbar::MultiProgressBar>();
    }
    auto progress_bar = std::make_unique<libdnf::cli::progressbar::DownloadProgressBar>(
        total_to_download > 0 ? total_to_download : -1, description);
    auto * ppb = progress_bar.get();
    multi_progress_bar->add_bar(std::move(progress_bar));
    return ppb;
}

int DownloadCallbacks::progress(void * user_cb_data, double total_to_download, double downloaded) {
    auto * progress_bar = reinterpret_cast<libdnf::cli::progressbar::DownloadProgressBar *>(user_cb_data);
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

int DownloadCallbacks::end(void * user_cb_data, TransferStatus status, const char * msg) {
    auto * progress_bar = reinterpret_cast<libdnf::cli::progressbar::DownloadProgressBar *>(user_cb_data);
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

int DownloadCallbacks::mirror_failure(void * user_cb_data, const char * msg, const char * url) {
    auto * progress_bar = reinterpret_cast<libdnf::cli::progressbar::DownloadProgressBar *>(user_cb_data);
    std::string message = std::string(msg) + " - " + url;
    progress_bar->add_message(libdnf::cli::progressbar::MessageType::ERROR, message);
    multi_progress_bar->print();
    return 0;
}

void DownloadCallbacks::reset_progress_bar() {
    multi_progress_bar.reset();
}

bool DownloadCallbacks::is_time_to_print() {
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

}  // namespace dnf5
