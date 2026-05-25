// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "test_offline.hpp"

#include <libdnf5/transaction/offline.hpp>
#include <libdnf5/utils/fs/file.hpp>

#include <filesystem>


using namespace libdnf5::offline;

CPPUNIT_TEST_SUITE_REGISTRATION(OfflineTransactionStateTest);


void OfflineTransactionStateTest::test_from_base_factory() {
    auto expected_path = base.get_config().get_installroot_option().get_value() / DEFAULT_DATADIR.relative_path() /
                         TRANSACTION_STATE_FILENAME;
    auto state = OfflineTransactionState::from_base(base);
    CPPUNIT_ASSERT_EQUAL(expected_path, state.get_path());
}


void OfflineTransactionStateTest::test_is_pending_no_file() {
    auto state = OfflineTransactionState::from_base(base);
    CPPUNIT_ASSERT(!state.is_pending());
    CPPUNIT_ASSERT(state.get_read_exception() != nullptr);
}


void OfflineTransactionStateTest::test_is_pending_download_incomplete() {
    auto state = OfflineTransactionState::from_base(base);
    auto & data = state.get_data();
    data.set_status(STATUS_DOWNLOAD_INCOMPLETE);
    auto dir = state.get_path().parent_path();
    std::filesystem::create_directories(dir);
    state.write();

    OfflineTransactionState state2(state.get_path());
    CPPUNIT_ASSERT(!state2.is_pending());
    CPPUNIT_ASSERT(state2.get_read_exception() == nullptr);
}


void OfflineTransactionStateTest::test_is_pending_download_complete() {
    auto state = OfflineTransactionState::from_base(base);
    auto & data = state.get_data();
    data.set_status(STATUS_DOWNLOAD_COMPLETE);
    auto dir = state.get_path().parent_path();
    std::filesystem::create_directories(dir);
    state.write();

    OfflineTransactionState state2(state.get_path());
    CPPUNIT_ASSERT(state2.is_pending());
    CPPUNIT_ASSERT(state2.get_read_exception() == nullptr);
}


void OfflineTransactionStateTest::test_invalidate() {
    auto state = OfflineTransactionState::from_base(base);
    auto & data = state.get_data();
    data.set_status(STATUS_DOWNLOAD_COMPLETE);
    auto dir = state.get_path().parent_path();
    std::filesystem::create_directories(dir);
    state.write();

    auto transaction_json = dir / "transaction.json";
    libdnf5::utils::fs::File(transaction_json, "w").close();

    auto packages_dir = dir.parent_path() / "packages";
    std::filesystem::create_directories(packages_dir);
    libdnf5::utils::fs::File(packages_dir / "test.rpm", "w").close();

    CPPUNIT_ASSERT(std::filesystem::exists(state.get_path()));
    CPPUNIT_ASSERT(std::filesystem::exists(transaction_json));

    state.invalidate();

    CPPUNIT_ASSERT(std::filesystem::exists(state.get_path()));
    CPPUNIT_ASSERT(std::filesystem::exists(transaction_json));
    CPPUNIT_ASSERT(std::filesystem::exists(packages_dir / "test.rpm"));

    OfflineTransactionState state2(state.get_path());
    CPPUNIT_ASSERT_EQUAL(STATUS_DOWNLOAD_COMPLETE, state2.get_data().get_status());
    CPPUNIT_ASSERT(state2.is_pending());
}
