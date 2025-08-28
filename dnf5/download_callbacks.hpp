// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DNF5_DOWNLOAD_CALLBACKS_HPP
#define DNF5_DOWNLOAD_CALLBACKS_HPP

#include <libdnf5-cli/progressbar/multi_progress_bar.hpp>
#include <libdnf5/repo/download_callbacks.hpp>

#include <chrono>

namespace dnf5 {

class DownloadCallbacks : public libdnf5::repo::DownloadCallbacks {
public:
    void set_number_widget_visible(bool value);

    void set_show_total_bar_limit(std::size_t limit);

    void reset_progress_bar();

private:
    void * add_new_download(void * user_data, const char * description, double total_to_download) override;

    int progress(void * user_cb_data, double total_to_download, double downloaded) override;

    int end(void * user_cb_data, TransferStatus status, const char * msg) override;

    int mirror_failure(void * user_cb_data, const char * msg, const char * url, const char * metadata) override;

    bool is_time_to_print();
    void print();

    std::unique_ptr<libdnf5::cli::progressbar::MultiProgressBar> multi_progress_bar;
    std::chrono::time_point<std::chrono::steady_clock> prev_print_time{std::chrono::steady_clock::now()};
    bool printed{false};

    bool number_widget_visible{false};
    std::size_t show_total_bar_limit{0};
};

}  // namespace dnf5

#endif
