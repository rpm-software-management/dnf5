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
