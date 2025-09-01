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

#include "test_file_downloader.hpp"

#include "utils/string.hpp"

#include <libdnf5/repo/file_downloader.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION(FileDownloaderTest);

namespace {

class DownloadCallbacks : public libdnf5::repo::DownloadCallbacks {
public:
    void * add_new_download(
        [[maybe_unused]] void * user_data,
        [[maybe_unused]] const char * description,
        [[maybe_unused]] double total_to_download) override {
        ++add_new_download_cnt;
        this->user_data = user_data;
        user_cb_data_holder = "User cb data";
        return &user_cb_data_holder;
    }

    int end([[maybe_unused]] void * user_cb_data, TransferStatus status, const char * msg) override {
        ++end_cnt;
        end_user_cb_data = static_cast<std::string *>(user_cb_data);
        end_status = status;
        end_msg = libdnf5::utils::string::c_to_str(msg);
        return 0;
    }

    int progress(
        [[maybe_unused]] void * user_cb_data,
        [[maybe_unused]] double total_to_download,
        [[maybe_unused]] double downloaded) override {
        ++progress_cnt;
        return 0;
    }

    int mirror_failure(
        [[maybe_unused]] void * user_cb_data,
        [[maybe_unused]] const char * msg,
        [[maybe_unused]] const char * url,
        [[maybe_unused]] const char * metadata) override {
        ++mirror_failure_cnt;
        return 0;
    }

    int add_new_download_cnt = 0;
    int progress_cnt = 0;
    int mirror_failure_cnt = 0;
    int end_cnt = 0;

    void * user_data = nullptr;
    std::string user_cb_data_holder;
    std::string * end_user_cb_data = nullptr;

    TransferStatus end_status;
    std::string end_msg;
};

}  // namespace


void FileDownloaderTest::test_file_downloader() {
    std::string user_data = "User data";

    std::string source_file_path = std::string(PROJECT_SOURCE_DIR) + "/test/data/keys/key.pub";
    std::string source_url = "file://" + source_file_path;
    auto dest_file_path = temp_dir->get_path() / "file_downloader.pub";

    auto cbs_unique_ptr = std::make_unique<DownloadCallbacks>();
    auto cbs = cbs_unique_ptr.get();
    base.set_download_callbacks(std::move(cbs_unique_ptr));

    libdnf5::repo::FileDownloader file_downloader(base);
    file_downloader.add(source_url, dest_file_path, &user_data);
    file_downloader.download();

    CPPUNIT_ASSERT_EQUAL(&user_data, static_cast<std::string *>(cbs->user_data));
    CPPUNIT_ASSERT_EQUAL(std::string("User cb data"), cbs->end_user_cb_data ? *cbs->end_user_cb_data : "");

    CPPUNIT_ASSERT_EQUAL(1, cbs->add_new_download_cnt);
    CPPUNIT_ASSERT_EQUAL(1, cbs->end_cnt);
    CPPUNIT_ASSERT_GREATEREQUAL(1, cbs->progress_cnt);
    CPPUNIT_ASSERT_EQUAL(0, cbs->mirror_failure_cnt);

    CPPUNIT_ASSERT_EQUAL(DownloadCallbacks::TransferStatus::SUCCESSFUL, cbs->end_status);
    CPPUNIT_ASSERT_EQUAL(std::string(), cbs->end_msg);

    // Check contents of downloaded file
    libdnf5::utils::fs::File source_file;
    source_file.open(source_file_path, "r");
    std::string source_file_content = source_file.read();
    libdnf5::utils::fs::File dest_file;
    dest_file.open(dest_file_path, "r");
    std::string dest_file_content = dest_file.read();
    CPPUNIT_ASSERT_EQUAL(source_file_content, dest_file_content);
}
