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

#include <filesystem>

CPPUNIT_TEST_SUITE_REGISTRATION(PackageDownloaderTest);


class DownloadCallbacks : public libdnf5::repo::DownloadCallbacks {
public:
    int end([[maybe_unused]] void * user_cb_data, TransferStatus status, const char * msg) override {
        ++end_cnt;
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

    int end_cnt = 0;
    TransferStatus end_status = TransferStatus::ERROR;
    std::string end_msg;

    int progress_cnt = 0;
    int mirror_failure_cnt = 0;
};

void PackageDownloaderTest::test_package_downloader() {
    auto repo = add_repo_rpm("rpm-repo1");

    libdnf5::rpm::PackageQuery query(base);
    query.filter_name("one");
    query.filter_version("2");
    query.filter_arch("noarch");
    CPPUNIT_ASSERT_EQUAL((size_t)1, query.size());

    auto downloader = libdnf5::repo::PackageDownloader(base);

    auto cbs_unique_ptr = std::make_unique<DownloadCallbacks>();
    auto cbs = cbs_unique_ptr.get();
    base.set_download_callbacks(std::move(cbs_unique_ptr));

    downloader.add(*query.begin());

    downloader.download();

    CPPUNIT_ASSERT_EQUAL(1, cbs->end_cnt);
    CPPUNIT_ASSERT_EQUAL(DownloadCallbacks::TransferStatus::SUCCESSFUL, cbs->end_status);
    CPPUNIT_ASSERT_EQUAL(std::string(""), cbs->end_msg);

    CPPUNIT_ASSERT_GREATEREQUAL(1, cbs->progress_cnt);
    CPPUNIT_ASSERT_EQUAL(0, cbs->mirror_failure_cnt);
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
    for (const auto & package : query) {
        downloader.add(package);
    }
    downloader.download();

    auto paths_prefix = std::filesystem::path(repo->get_cachedir()) / std::filesystem::path("packages");
    const std::vector<std::string> expected = {
        paths_prefix / "one-1-1.noarch.rpm",
        paths_prefix / "one-1-1.src.rpm",
        paths_prefix / "one-2-1.noarch.rpm",
        paths_prefix / "one-2-1.src.rpm"};

    CPPUNIT_ASSERT_EQUAL(expected, memory.get_files());
}
