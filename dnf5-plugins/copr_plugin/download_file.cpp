// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "download_file.hpp"

#include <libdnf5/repo/file_downloader.hpp>

#include <filesystem>

void download_file(libdnf5::Base & base, const std::string & url, const std::filesystem::path & path) {
    libdnf5::repo::FileDownloader downloader(base);
    downloader.add(url, path);
    downloader.download();
}
