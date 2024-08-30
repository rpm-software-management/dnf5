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

#include "libdnf5/repo/download_callbacks.hpp"

namespace libdnf5::repo {

void * DownloadCallbacks::add_new_download(
    [[maybe_unused]] void * user_data,
    [[maybe_unused]] const char * description,
    [[maybe_unused]] double total_to_download) {
    return nullptr;
}

int DownloadCallbacks::end(
    [[maybe_unused]] void * user_cb_data, [[maybe_unused]] TransferStatus status, [[maybe_unused]] const char * msg) {
    return ReturnCode::OK;
}

int DownloadCallbacks::progress(
    [[maybe_unused]] void * user_cb_data,
    [[maybe_unused]] double total_to_download,
    [[maybe_unused]] double downloaded) {
    return ReturnCode::OK;
}

int DownloadCallbacks::mirror_failure(
    [[maybe_unused]] void * user_cb_data,
    [[maybe_unused]] const char * msg,
    [[maybe_unused]] const char * url,
    [[maybe_unused]] const char * metadata) {
    return ReturnCode::OK;
}

void DownloadCallbacks::fastest_mirror(
    [[maybe_unused]] void * user_cb_data,
    [[maybe_unused]] FastestMirrorStage stage,
    [[maybe_unused]] const char * ptr) {}

}  // namespace libdnf5::repo
