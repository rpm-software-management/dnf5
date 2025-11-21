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


#include "test_transaction.hpp"

#include "../shared/utils.hpp"
#include "utils/string.hpp"

#include <libdnf5/base/base.hpp>
#include <libdnf5/base/goal.hpp>
#include <libdnf5/base/transaction_package.hpp>
#include <libdnf5/repo/package_downloader.hpp>
#include <libdnf5/rpm/transaction_callbacks.hpp>

#include <filesystem>


CPPUNIT_TEST_SUITE_REGISTRATION(RpmTransactionTest);


using namespace libdnf5::rpm;
using namespace libdnf5::transaction;


namespace {

class PackageDownloadCallbacks : public libdnf5::repo::DownloadCallbacks {
public:
    int end(
        [[maybe_unused]] void * user_cb_data,
        libdnf5::repo::DownloadCallbacks::TransferStatus status,
        const char * msg) override {
        ++end_cnt;
        end_status = status;
        end_msg = msg ? msg : "";
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

    int end_cnt{0};
    TransferStatus end_status{TransferStatus::ERROR};
    std::string end_msg;

    int progress_cnt{0};
    int mirror_failure_cnt{0};
};

class OrderCapturingCallbacks : public libdnf5::rpm::TransactionCallbacks {
public:
    explicit OrderCapturingCallbacks(std::vector<std::string> expected) : expected_order(std::move(expected)) {}

    void install_start(const libdnf5::base::TransactionPackage & item, [[maybe_unused]] uint64_t total) override {
        auto nevra = item.get_package().get_nevra();
        if (current_index >= expected_order.size() || nevra != expected_order[current_index]) {
            order_correct = false;
        }
        current_index++;
    }

    std::vector<std::string> expected_order;
    size_t current_index = 0;
    bool order_correct = true;
};

}  // namespace


void RpmTransactionTest::test_transaction() {
    add_repo_rpm("rpm-repo1");

    libdnf5::Goal goal(base);

    goal.add_rpm_install("one");

    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {libdnf5::base::TransactionPackage(
        get_pkg("one-0:2-1.noarch"),
        TransactionItemAction::INSTALL,
        TransactionItemReason::USER,
        TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());

    auto dl_callbacks = std::make_unique<PackageDownloadCallbacks>();
    auto dl_callbacks_ptr = dl_callbacks.get();
    base.set_download_callbacks(std::move(dl_callbacks));

    transaction.download();

    CPPUNIT_ASSERT_EQUAL(1, dl_callbacks_ptr->end_cnt);
    CPPUNIT_ASSERT_EQUAL(PackageDownloadCallbacks::TransferStatus::SUCCESSFUL, dl_callbacks_ptr->end_status);
    CPPUNIT_ASSERT_EQUAL(std::string(), dl_callbacks_ptr->end_msg);

    CPPUNIT_ASSERT_GREATEREQUAL(1, dl_callbacks_ptr->progress_cnt);
    CPPUNIT_ASSERT_EQUAL(0, dl_callbacks_ptr->mirror_failure_cnt);

    // TODO(lukash) test transaction callbacks
    transaction.set_callbacks(std::make_unique<libdnf5::rpm::TransactionCallbacks>());
    transaction.set_description("install package one");

    // TODO(jkolarik): Temporarily disable the test to allow further investigation of issues on the RISC-V arch
    //                 See https://github.com/rpm-software-management/dnf5/issues/503
    auto res = transaction.run();
    if (res != libdnf5::base::Transaction::TransactionRunResult::SUCCESS) {
        std::cout << std::endl << "WARNING: Transaction was not successful" << std::endl;
        std::cout << libdnf5::utils::string::join(transaction.get_transaction_problems(), ", ") << std::endl;
    }

    // TODO(lukash) assert the packages were installed
}

void RpmTransactionTest::test_transaction_temp_files_cleanup() {
    auto repo = add_repo_rpm("rpm-repo1");

    // ensure keepcache option is switched off
    base.get_config().get_keepcache_option().set(false);

    libdnf5::Goal goal(base);
    goal.add_rpm_install("one");

    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {libdnf5::base::TransactionPackage(
        get_pkg("one-0:2-1.noarch"),
        TransactionItemAction::INSTALL,
        TransactionItemReason::USER,
        TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());

    auto package_path =
        std::filesystem::path(repo->get_cachedir()) / std::filesystem::path("packages/one-2-1.noarch.rpm");

    // check package was downloaded and then removed after transaction run
    CPPUNIT_ASSERT(!std::filesystem::exists(package_path));
    transaction.download();
    CPPUNIT_ASSERT(std::filesystem::exists(package_path));

    // TODO(jkolarik): Temporarily disable the test to allow further investigation of issues on the RISC-V arch
    //                 See https://github.com/rpm-software-management/dnf5/issues/503
    auto res = transaction.run();
    if (res != libdnf5::base::Transaction::TransactionRunResult::SUCCESS) {
        std::cout << std::endl << "WARNING: Transaction was not successful" << std::endl;
        std::cout << libdnf5::utils::string::join(transaction.get_transaction_problems(), ", ") << std::endl;
    } else {
        CPPUNIT_ASSERT(!std::filesystem::exists(package_path));
    }
}

void RpmTransactionTest::test_source_date_epoch_sorting() {
    add_repo_rpm("rpm-repo1", /* load */ false);
    add_repo_rpm("rpm-repo2", /* load */ false);
    add_repo_rpm("rpm-repo3", /* load */ true);

    // setting a global var in a test is not great... no other tests right now
    // relate to SOURCE_DATE_EPOCH at least
    setenv("SOURCE_DATE_EPOCH", "1234567890", 1);

    std::vector<std::string> packages = {"one", "two", "three"};
    // The exact final install order chosen by librpm doesn't matter. What
    // matters is that it's consistent across all permutations. An alternative
    // approach resilient to potential librpm changes is to capture the order on
    // the first iteration and then compare future iterations against that.
    std::vector<std::string> expected_order = {"two-2-2.noarch", "three-1-1.noarch", "one-2-1.noarch"};
    auto installroot = base.get_config().get_installroot_option().get_value();

    std::sort(packages.begin(), packages.end());
    do {
        // recreate the installroot each time so we start from scratch
        std::filesystem::remove_all(installroot);
        std::filesystem::create_directory(installroot);

        libdnf5::Goal goal(base);
        for (const auto & pkg : packages) {
            goal.add_rpm_install(pkg);
        }

        auto transaction = goal.resolve();
        transaction.download();

        auto callbacks = std::make_unique<OrderCapturingCallbacks>(expected_order);
        auto callbacks_ptr = callbacks.get();
        transaction.set_callbacks(std::move(callbacks));

        // TODO(jkolarik): Temporarily disable the test to allow further investigation of issues on the RISC-V arch
        //                 See https://github.com/rpm-software-management/dnf5/issues/503
        auto res = transaction.run();
        if (res != libdnf5::base::Transaction::TransactionRunResult::SUCCESS) {
            std::cout << std::endl << "WARNING: Transaction was not successful" << std::endl;
            std::cout << libdnf5::utils::string::join(transaction.get_transaction_problems(), ", ") << std::endl;
            break;  // no point in trying more permutations if arch is broken
        }

        CPPUNIT_ASSERT(callbacks_ptr->order_correct);
        CPPUNIT_ASSERT_EQUAL(expected_order.size(), callbacks_ptr->current_index);

    } while (std::next_permutation(packages.begin(), packages.end()));

    unsetenv("SOURCE_DATE_EPOCH");
}
