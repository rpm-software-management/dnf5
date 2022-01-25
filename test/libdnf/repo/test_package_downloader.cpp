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

#include "utils/string.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/repo/package_downloader.hpp"
#include "libdnf/rpm/package_query.hpp"


CPPUNIT_TEST_SUITE_REGISTRATION(PackageDownloaderTest);


class PackageDownloadCallbacks : public libdnf::repo::PackageDownloadCallbacks {
public:
    int end(TransferStatus status, const char * msg) override {
        ++end_cnt;
        end_status = status;
        end_msg = libdnf::utils::string::c_to_str(msg);
        return 0;
    }

    int progress([[maybe_unused]] double total_to_download, [[maybe_unused]] double downloaded) override {
        ++progress_cnt;
        return 0;
    }

    int mirror_failure([[maybe_unused]] const char * msg, [[maybe_unused]] const char * url) override {
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

    libdnf::rpm::PackageQuery query(base);
    query.filter_name({"one"}).filter_version({"2"}).filter_arch({"noarch"});
    CPPUNIT_ASSERT_EQUAL(1lu, query.size());

    auto downloader = libdnf::repo::PackageDownloader();

    auto cbs_unique_ptr = std::make_unique<PackageDownloadCallbacks>();
    auto cbs = cbs_unique_ptr.get();
    downloader.add(*query.begin(), std::move(cbs_unique_ptr));

    downloader.download(true, true);

    CPPUNIT_ASSERT_EQUAL(1, cbs->end_cnt);
    CPPUNIT_ASSERT_EQUAL(PackageDownloadCallbacks::TransferStatus::SUCCESSFUL, cbs->end_status);
    CPPUNIT_ASSERT_EQUAL(std::string(""), cbs->end_msg);

    CPPUNIT_ASSERT_GREATEREQUAL(1, cbs->progress_cnt);
    CPPUNIT_ASSERT_EQUAL(0, cbs->mirror_failure_cnt);
}
