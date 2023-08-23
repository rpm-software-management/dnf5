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

#include "download_callbacks_simple.hpp"

#include <iostream>
#include <sstream>

namespace dnf5 {

void * DownloadCallbacksSimple::add_new_download(
    [[maybe_unused]] void * user_data, const char * description, [[maybe_unused]] double total_to_download) {
    // We cannot print the download description right here, because for packages
    // the `add_new_download` is called for each package before the download
    // actually starts. So just store the description to print later in `end` callback
    // together with download status.
    return &active_downloads.emplace_front(description);
}

int DownloadCallbacksSimple::end(void * user_cb_data, TransferStatus status, const char * msg) {
    // check that user_cb_data is really present in active_downloads.
    std::string * description{nullptr};
    for (const auto & item : active_downloads) {
        if (&item == user_cb_data) {
            description = reinterpret_cast<std::string *>(user_cb_data);
            break;
        }
    }
    if (!description) {
        return 0;
    }

    // print the download status of the item
    std::string message;
    switch (status) {
        case TransferStatus::SUCCESSFUL:
            output_stream << "  Downloaded: " << *description << std::endl;
            break;
        case TransferStatus::ALREADYEXISTS:
            output_stream << "  Already downloaded: " << *description << std::endl;
            break;
        case TransferStatus::ERROR:
            output_stream << "  Error downloading: " << *description << ": " << msg << std::endl;
            break;
    }

    // remove the finished item from the active_downloads list
    active_downloads.remove_if([description](const std::string & item) { return &item == description; });
    return 0;
}

}  // namespace dnf5
