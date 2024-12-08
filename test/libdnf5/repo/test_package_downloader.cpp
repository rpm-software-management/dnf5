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

#include "test_package_downloader.hpp"

#include "../shared/utils.hpp"
#include "repo/temp_files_memory.hpp"
#include "utils/string.hpp"

#include "libdnf5/utils/fs/file.hpp"

#include <libdnf5/base/base.hpp>
#include <libdnf5/repo/package_downloader.hpp>
#include <libdnf5/rpm/package_query.hpp>

#include <algorithm>
#include <filesystem>

CPPUNIT_TEST_SUITE_REGISTRATION(PackageDownloaderTest);

namespace {

class DownloadCallbacks : public libdnf5::repo::DownloadCallbacks {
public:
    void * add_new_download(
        [[maybe_unused]] void * user_data,
        [[maybe_unused]] const char * description,
        [[maybe_unused]] double total_to_download) override {
        ++add_new_download_cnt;
        user_data_array.emplace_back(user_data);
        std::string user_cb_data = std::string("Package: ") + description;
        return user_cb_data_container.emplace_back(std::move(user_cb_data)).data();
    }

    int end([[maybe_unused]] void * user_cb_data, TransferStatus status, const char * msg) override {
        ++end_cnt;
        user_cb_data_array.emplace_back(static_cast<const char *>(user_cb_data));
        end_status.emplace_back(status);
        end_msg.emplace_back(libdnf5::utils::string::c_to_str(msg));
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

    std::vector<std::string> user_cb_data_container;

    int add_new_download_cnt = 0;
    int progress_cnt = 0;
    int mirror_failure_cnt = 0;
    int end_cnt = 0;

    std::vector<void *> user_data_array;
    std::vector<const char *> user_cb_data_array;
    std::vector<TransferStatus> end_status;
    std::vector<std::string> end_msg;
};

}  // namespace


void PackageDownloaderTest::test_package_downloader() {
    auto repo = add_repo_rpm("rpm-repo1");

    libdnf5::rpm::PackageQuery query(base);
    query.filter_name("one");
    query.filter_arch("noarch");
    CPPUNIT_ASSERT_EQUAL((size_t)2, query.size());

    auto downloader = libdnf5::repo::PackageDownloader(base);

    auto cbs_unique_ptr = std::make_unique<DownloadCallbacks>();
    auto cbs = cbs_unique_ptr.get();
    base.set_download_callbacks(std::move(cbs_unique_ptr));

    std::string user_data = "User data";
    for (const auto & package : query) {
        downloader.add(package, &user_data);
    }

    downloader.download();

    std::sort(cbs->user_cb_data_container.begin(), cbs->user_cb_data_container.end());
    CPPUNIT_ASSERT_EQUAL(
        (std::vector<std::string>{"Package: one-0:1-1.noarch", "Package: one-0:2-1.noarch"}),
        cbs->user_cb_data_container);

    CPPUNIT_ASSERT_EQUAL(2, cbs->add_new_download_cnt);
    CPPUNIT_ASSERT_EQUAL(2, cbs->end_cnt);
    CPPUNIT_ASSERT_GREATEREQUAL(2, cbs->progress_cnt);
    CPPUNIT_ASSERT_EQUAL(0, cbs->mirror_failure_cnt);

    for (auto * item : cbs->user_data_array) {
        CPPUNIT_ASSERT_EQUAL(&user_data, static_cast<std::string *>(item));
    }

    std::sort(cbs->user_cb_data_array.begin(), cbs->user_cb_data_array.end(), [](const char * a, const char * b) {
        return std::string_view(a).compare(b) < 0;
    });
    CPPUNIT_ASSERT_EQUAL(cbs->user_cb_data_container[0].c_str(), cbs->user_cb_data_array[0]);
    CPPUNIT_ASSERT_EQUAL(cbs->user_cb_data_container[1].c_str(), cbs->user_cb_data_array[1]);

    CPPUNIT_ASSERT_EQUAL(
        (std::vector<DownloadCallbacks::TransferStatus>{
            DownloadCallbacks::TransferStatus::SUCCESSFUL, DownloadCallbacks::TransferStatus::SUCCESSFUL}),
        cbs->end_status);
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"", ""}), cbs->end_msg);
}

void PackageDownloaderTest::test_package_downloader_temp_files_memory() {
    auto repo = add_repo_rpm("rpm-repo1");

    libdnf5::rpm::PackageQuery query(base);
    query.filter_name("one");
    CPPUNIT_ASSERT_EQUAL((size_t)4, query.size());

    auto & config = base.get_config();

    // ensure keepcache option is switched off
    config.get_keepcache_option().set(false);

    auto & cachedir = config.get_cachedir_option().get_value();
    libdnf5::repo::TempFilesMemory memory(base.get_weak_ptr(), cachedir);

    // check memory is empty before downloading
    CPPUNIT_ASSERT_EQUAL((size_t)0, memory.get_files().size());

    auto downloader = libdnf5::repo::PackageDownloader(base);

    auto cbs_unique_ptr = std::make_unique<DownloadCallbacks>();
    auto cbs = cbs_unique_ptr.get();
    base.set_download_callbacks(std::move(cbs_unique_ptr));

    for (const auto & package : query) {
        downloader.add(package);
    }
    downloader.download();

    CPPUNIT_ASSERT_EQUAL(4, cbs->add_new_download_cnt);
    CPPUNIT_ASSERT_EQUAL(4, cbs->end_cnt);
    CPPUNIT_ASSERT_GREATEREQUAL(4, cbs->progress_cnt);
    CPPUNIT_ASSERT_EQUAL(0, cbs->mirror_failure_cnt);

    auto paths_prefix = std::filesystem::path(repo->get_cachedir()) / std::filesystem::path("packages");
    const std::vector<std::string> expected = {
        paths_prefix / "one-1-1.noarch.rpm",
        paths_prefix / "one-1-1.src.rpm",
        paths_prefix / "one-2-1.noarch.rpm",
        paths_prefix / "one-2-1.src.rpm"};

    CPPUNIT_ASSERT_EQUAL(expected, memory.get_files());
}
