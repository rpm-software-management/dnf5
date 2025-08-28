// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DNF5_PLUGINS_AUTOMATIC_PLUGIN_DOWNLOAD_CALLBACKS_SIMPLE_HPP
#define DNF5_PLUGINS_AUTOMATIC_PLUGIN_DOWNLOAD_CALLBACKS_SIMPLE_HPP

#include <libdnf5/repo/download_callbacks.hpp>

#include <forward_list>
#include <sstream>
#include <string>

namespace dnf5 {

/// Simple callbacks class. It does not print any progressbars, only
/// the result of the download.
class DownloadCallbacksSimple : public libdnf5::repo::DownloadCallbacks {
public:
    explicit DownloadCallbacksSimple(std::stringstream & output_stream) : output_stream(output_stream) {}

private:
    void * add_new_download(void * user_data, const char * description, double total_to_download) override;

    int end(void * user_cb_data, TransferStatus status, const char * msg) override;

    /// keeps list of descriptions of currently active downloads
    std::forward_list<std::string> active_downloads;

    std::stringstream & output_stream;
};

}  // namespace dnf5

#endif
