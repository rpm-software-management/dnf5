// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

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
        multi_progress_bar = std::make_unique<libdnf5::cli::progressbar::MultiProgressBar>(
            libdnf5::cli::progressbar::MultiProgressBar::TrackingMode::ON_CHANGE);
        multi_progress_bar->set_total_bar_visible_limit(show_total_bar_limit);
    }
    auto progress_bar = std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(
        total_to_download > 0 ? total_to_download : -1, description);
    auto * ppb = progress_bar.get();
    ppb->set_number_widget_visible(number_widget_visible);
    ppb->set_auto_finish(false);
    multi_progress_bar->add_bar(std::move(progress_bar));
    ++num_of_downloads;
    return ppb;
}

int DownloadCallbacks::progress(void * user_cb_data, double total_to_download, double downloaded) {
    auto & progress_bar = *reinterpret_cast<libdnf5::cli::progressbar::DownloadProgressBar *>(user_cb_data);
    auto total = static_cast<int64_t>(total_to_download);
    if (total > 0) {
        multi_progress_bar->bar_set_total_ticks(progress_bar, total);
    }
    if (multi_progress_bar->bar_get_state(progress_bar) == libdnf5::cli::progressbar::ProgressBarState::READY) {
        multi_progress_bar->bar_start(progress_bar);
    }
    multi_progress_bar->bar_set_ticks(progress_bar, static_cast<int64_t>(downloaded));
    if (is_time_to_print()) {
        print();
    }
    return ReturnCode::OK;
}

int DownloadCallbacks::end(void * user_cb_data, TransferStatus status, const char * msg) {
    auto & progress_bar = *reinterpret_cast<libdnf5::cli::progressbar::DownloadProgressBar *>(user_cb_data);
    switch (status) {
        case TransferStatus::SUCCESSFUL:
            // Correction of the total data size for the download.
            // Sometimes Librepo returns a larger data size for download than the actual file size.
            multi_progress_bar->bar_set_total_ticks(progress_bar, multi_progress_bar->bar_get_ticks(progress_bar));

            multi_progress_bar->bar_set_state(progress_bar, libdnf5::cli::progressbar::ProgressBarState::SUCCESS);
            break;
        case TransferStatus::ALREADYEXISTS:
            // skipping the download -> downloading 0 bytes
            multi_progress_bar->bar_set_ticks(progress_bar, 0);
            multi_progress_bar->bar_set_total_ticks(progress_bar, 0);
            multi_progress_bar->bar_add_message(progress_bar, libdnf5::cli::progressbar::MessageType::SUCCESS, msg);
            multi_progress_bar->bar_start(progress_bar);
            multi_progress_bar->bar_set_state(progress_bar, libdnf5::cli::progressbar::ProgressBarState::SUCCESS);
            break;
        case TransferStatus::ERROR:
            multi_progress_bar->bar_add_message(progress_bar, libdnf5::cli::progressbar::MessageType::ERROR, msg);
            multi_progress_bar->bar_set_state(progress_bar, libdnf5::cli::progressbar::ProgressBarState::ERROR);
            break;
    }
    ++num_of_ended_downloads;
    // Rate limit the repaint the same way progress() does. Repainting once per
    // completed download is what makes downloading many small packages (a
    // reposync of a whole repository, for example) expensive: every repaint
    // re-renders each bar and rewrites the whole block of lines, so the cost
    // grows with the number of packages rather than with the time spent.
    //
    // Nothing is lost by waiting - a repaint prints every bar that has finished
    // since the previous one, in order - and the last download always repaints,
    // so the final state of every bar reaches the terminal. libdnf5 guarantees
    // exactly one end() call per add_new_download(), including for downloads
    // that were interrupted, so the counters cannot drift.
    if (all_downloads_ended() || is_time_to_print()) {
        print();
    }
    return ReturnCode::OK;
}

int DownloadCallbacks::mirror_failure(void * user_cb_data, const char * msg, const char * url, const char * metadata) {
    auto & progress_bar = *reinterpret_cast<libdnf5::cli::progressbar::DownloadProgressBar *>(user_cb_data);
    std::string message = std::string(msg) + " - " + url;
    if (metadata) {
        message = message + " - " + metadata;
    }
    multi_progress_bar->bar_add_message(progress_bar, libdnf5::cli::progressbar::MessageType::ERROR, message);
    // The message is attached to the bar, so it is printed by the next repaint.
    // A failing mirror is always followed by an end() call for the same target,
    // which repaints if this one didn't.
    if (is_time_to_print()) {
        print();
    }
    return ReturnCode::OK;
}

void DownloadCallbacks::reset_progress_bar() {
    multi_progress_bar.reset();
    num_of_downloads = 0;
    num_of_ended_downloads = 0;
    if (printed) {
        printed = false;
    }
}

bool DownloadCallbacks::all_downloads_ended() const noexcept {
    return num_of_ended_downloads >= num_of_downloads;
}

bool DownloadCallbacks::is_time_to_print() const noexcept {
    auto delta = std::chrono::steady_clock::now() - prev_print_time;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
    // 100ms equals to 10 FPS and that seems to be smooth enough
    return ms > 100;
}

void DownloadCallbacks::print() {
    multi_progress_bar->print();
    prev_print_time = std::chrono::steady_clock::now();
    printed = true;
}

}  // namespace dnf5
