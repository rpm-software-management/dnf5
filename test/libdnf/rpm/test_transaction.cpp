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

#include "libdnf/base/goal.hpp"
#include "libdnf/repo/package_downloader.hpp"
#include "libdnf/rpm/transaction.hpp"


CPPUNIT_TEST_SUITE_REGISTRATION(RpmTransactionTest);


namespace {

// make constructor public so we can create instances in the tests
class TransactionPackage : public libdnf::base::TransactionPackage {
public:
    TransactionPackage(const libdnf::rpm::Package & pkg, Action action, Reason reason, State state)
        : libdnf::base::TransactionPackage(pkg, action, reason) {
        this->state = state;
    }
};

}  // namespace


using namespace libdnf::rpm;


class PackageDownloadCallbacks : public libdnf::repo::PackageDownloadCallbacks {
public:
    int end(libdnf::repo::PackageDownloadCallbacks::TransferStatus status, const char * msg) override {
        ++end_cnt;
        end_status = status;
        end_msg = msg ? msg : "";
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

    int end_cnt{0};
    TransferStatus end_status{TransferStatus::ERROR};
    std::string end_msg;

    int progress_cnt{0};
    int mirror_failure_cnt{0};
};


void RpmTransactionTest::test_transaction() {
    add_repo_rpm("rpm-repo1");

    libdnf::Goal goal(base);

    goal.add_rpm_install("one");

    auto transaction = goal.resolve(false);

    std::vector<libdnf::base::TransactionPackage> expected = {TransactionPackage(
        get_pkg("one-0:2-1.noarch"),
        libdnf::transaction::TransactionItemAction::INSTALL,
        libdnf::transaction::TransactionItemReason::USER,
        libdnf::transaction::TransactionItemState::UNKNOWN)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());

    libdnf::repo::PackageDownloader downloader;

    auto dl_callbacks = std::make_unique<PackageDownloadCallbacks>();
    auto dl_callbacks_ptr = dl_callbacks.get();

    for (auto & tspkg : transaction.get_transaction_packages()) {
        if (transaction_item_action_is_inbound(tspkg.get_action())) {
            downloader.add(tspkg.get_package(), std::move(dl_callbacks));
        }
    }

    downloader.download(true, true);

    CPPUNIT_ASSERT_EQUAL(1, dl_callbacks_ptr->end_cnt);
    CPPUNIT_ASSERT_EQUAL(PackageDownloadCallbacks::TransferStatus::SUCCESSFUL, dl_callbacks_ptr->end_status);
    CPPUNIT_ASSERT_EQUAL(std::string(), dl_callbacks_ptr->end_msg);

    CPPUNIT_ASSERT_GREATEREQUAL(1, dl_callbacks_ptr->progress_cnt);
    CPPUNIT_ASSERT_EQUAL(0, dl_callbacks_ptr->mirror_failure_cnt);

    // TODO(lukash) test transaction callbacks
    libdnf::base::Transaction::TransactionRunResult res = transaction.run(
        std::make_unique<libdnf::rpm::TransactionCallbacks>(), "install package one", std::nullopt, std::nullopt);

    CPPUNIT_ASSERT_EQUAL(libdnf::base::Transaction::TransactionRunResult::SUCCESS, res);
    // TODO(lukash) assert the packages were installed
}
