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

void DownloadCallbacks::set_number_widget_visible(bool value) {
    number_widget_visible = value;
}

void DownloadCallbacks::set_show_total_bar_limit(std::size_t limit) {
    show_total_bar_limit = limit;
    if (multi_progress_bar) {
        multi_progress_bar->set_total_bar_visible_limit(limit);
    }
}

void * DownloadCallbacks::add_new_download(
    [[maybe_unused]] void * user_data, const char * description, double total_to_download) {
    if (!multi_progress_bar) {
        multi_progress_bar = std::make_unique<libdnf5::cli::progressbar::MultiProgressBar>();
        multi_progress_bar->set_total_bar_visible_limit(show_total_bar_limit);
    }
    auto progress_bar = std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(
        total_to_download > 0 ? total_to_download : -1, description);
    auto * ppb = progress_bar.get();
    ppb->set_number_widget_visible(number_widget_visible);
    ppb->set_auto_finish(false);
    multi_progress_bar->add_bar(std::move(progress_bar));
    return ppb;
}

int DownloadCallbacks::progress(void * user_cb_data, double total_to_download, double downloaded) {
    auto * progress_bar = reinterpret_cast<libdnf5::cli::progressbar::DownloadProgressBar *>(user_cb_data);
    auto total = static_cast<int64_t>(total_to_download);
    if (total > 0) {
        progress_bar->set_total_ticks(total);
    }
    if (progress_bar->get_state() == libdnf5::cli::progressbar::ProgressBarState::READY) {
        progress_bar->start();
    }
    progress_bar->set_ticks(static_cast<int64_t>(downloaded));
    if (is_time_to_print()) {
        print();
    }
    return ReturnCode::OK;
}

int DownloadCallbacks::end(void * user_cb_data, TransferStatus status, const char * msg) {
    auto * progress_bar = reinterpret_cast<libdnf5::cli::progressbar::DownloadProgressBar *>(user_cb_data);
    switch (status) {
        case TransferStatus::SUCCESSFUL:
            // Correction of the total data size for the download.
            // Sometimes Librepo returns a larger data size for download than the actual file size.
            progress_bar->set_total_ticks(progress_bar->get_ticks());

            progress_bar->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);
            break;
        case TransferStatus::ALREADYEXISTS:
            // skipping the download -> downloading 0 bytes
            progress_bar->set_ticks(0);
            progress_bar->set_total_ticks(0);
            progress_bar->add_message(libdnf5::cli::progressbar::MessageType::SUCCESS, msg);
            progress_bar->start();
            progress_bar->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);
            break;
        case TransferStatus::ERROR:
            progress_bar->add_message(libdnf5::cli::progressbar::MessageType::ERROR, msg);
            progress_bar->set_state(libdnf5::cli::progressbar::ProgressBarState::ERROR);
            break;
    }
    print();
    return ReturnCode::OK;
}

int DownloadCallbacks::mirror_failure(void * user_cb_data, const char * msg, const char * url, const char * metadata) {
    auto * progress_bar = reinterpret_cast<libdnf5::cli::progressbar::DownloadProgressBar *>(user_cb_data);
    std::string message = std::string(msg) + " - " + url;
    if (metadata) {
        message = message + " - " + metadata;
    }
    progress_bar->add_message(libdnf5::cli::progressbar::MessageType::ERROR, message);
    print();
    return ReturnCode::OK;
}

void DownloadCallbacks::reset_progress_bar() {
    multi_progress_bar.reset();
    if (printed) {
        std::cerr << std::endl;
        printed = false;
    }
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

void DownloadCallbacks::print() {
    multi_progress_bar->print();
    printed = true;
}

}  // namespace dnf5
