// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
