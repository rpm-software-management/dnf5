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


#include "test_transaction.hpp"

#include "utils.hpp"
#include "utils/string.hpp"

#include "libdnf/base/goal.hpp"
#include "libdnf/base/transaction_package.hpp"
#include "libdnf/repo/package_downloader.hpp"
#include "libdnf/rpm/transaction_callbacks.hpp"


CPPUNIT_TEST_SUITE_REGISTRATION(RpmTransactionTest);


using namespace libdnf::rpm;
using namespace libdnf::transaction;


class CallbacksStats {
public:
    int end_cnt = 0;
    libdnf::repo::DownloadCallbacks::TransferStatus end_status = libdnf::repo::DownloadCallbacks::TransferStatus::ERROR;
    std::string end_msg;

    int progress_cnt = 0;
    int mirror_failure_cnt = 0;
};

class DownloadCallbacks : public libdnf::repo::DownloadCallbacks {
public:
    DownloadCallbacks(CallbacksStats * stats) : stats(stats) {}
    int end(TransferStatus status, const char * msg) override {
        ++stats->end_cnt;
        stats->end_status = status;
        stats->end_msg = libdnf::utils::string::c_to_str(msg);
        return 0;
    }

    int progress([[maybe_unused]] double total_to_download, [[maybe_unused]] double downloaded) override {
        ++stats->progress_cnt;
        return 0;
    }

    int mirror_failure([[maybe_unused]] const char * msg, [[maybe_unused]] const char * url) override {
        ++stats->mirror_failure_cnt;
        return 0;
    }

    CallbacksStats * stats;
};

class DownloadCallbacksFactory : public libdnf::repo::DownloadCallbacksFactory {
public:
    DownloadCallbacksFactory(CallbacksStats * stats) : stats(stats) {}
    std::unique_ptr<libdnf::repo::DownloadCallbacks> create_callbacks(
        [[maybe_unused]] const libdnf::rpm::Package & package) override {
        return std::make_unique<DownloadCallbacks>(stats);
    }
    CallbacksStats * stats;
};


void RpmTransactionTest::test_transaction() {
    add_repo_rpm("rpm-repo1");

    libdnf::Goal goal(base);

    goal.add_rpm_install("one");

    auto transaction = goal.resolve();

    std::vector<libdnf::base::TransactionPackage> expected = {libdnf::base::TransactionPackage(
        get_pkg("one-0:2-1.noarch"),
        TransactionItemAction::INSTALL,
        TransactionItemReason::USER,
        TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());

    CallbacksStats stats;
    auto downloader = libdnf::repo::PackageDownloader(base.get_weak_ptr());
    base.set_download_callbacks_factory(std::make_unique<DownloadCallbacksFactory>(&stats));

    for (auto & tspkg : transaction.get_transaction_packages()) {
        if (transaction_item_action_is_inbound(tspkg.get_action())) {
            downloader.add(tspkg.get_package());
        }
    }

    downloader.download(true, true);

    CPPUNIT_ASSERT_EQUAL(1, stats.end_cnt);
    CPPUNIT_ASSERT_EQUAL(DownloadCallbacks::TransferStatus::SUCCESSFUL, stats.end_status);
    CPPUNIT_ASSERT_EQUAL(std::string(), stats.end_msg);

    CPPUNIT_ASSERT_GREATEREQUAL(1, stats.progress_cnt);
    CPPUNIT_ASSERT_EQUAL(0, stats.mirror_failure_cnt);

    // TODO(lukash) test transaction callbacks
    libdnf::base::Transaction::TransactionRunResult res = transaction.run(
        std::make_unique<libdnf::rpm::TransactionCallbacks>(), "install package one", std::nullopt, std::nullopt);

    CPPUNIT_ASSERT_EQUAL(libdnf::base::Transaction::TransactionRunResult::SUCCESS, res);
    // TODO(lukash) assert the packages were installed
}
