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

// TODO(dmach): keep refactoring and deliver something that works with the new code base
// the whole file is disabled via the SKIP macro because it doesn't compile with the new code
#ifdef SKIP

#include "TransactionItemReasonTest.hpp"

#include <libdnf5/transaction/Swdb.hpp>
#include <libdnf5/transaction/Transformer.hpp>
#include <libdnf5/transaction/comps_environment.hpp>
#include <libdnf5/transaction/comps_group.hpp>
#include <libdnf5/transaction/rpm_package.hpp>
#include <libdnf5/transaction/transaction.hpp>
#include <libdnf5/transaction/transaction_item.hpp>

#include <cstdio>
#include <iostream>
#include <string>

using namespace libdnf5::transaction;

CPPUNIT_TEST_SUITE_REGISTRATION(TransactionItemReasonTest);

void TransactionItemReasonTest::setUp() {
    conn = new libdnf5::utils::SQLite3(":memory:");
    Transformer::createDatabase(*conn);
}

void TransactionItemReasonTest::tearDown() {
    delete conn;
}

// no transactions -> UNKNOWN reason
void TransactionItemReasonTest::testNoTransaction() {
    Swdb swdb(*conn);

    CPPUNIT_ASSERT_EQUAL(
        static_cast<TransactionItemReason>(swdb.resolveRPMTransactionItemReason("bash", "x86_64", -1)),
        TransactionItemReason::UNKNOWN);
}

// one transaction with no transaction items -> UNKNOWN reason
void TransactionItemReasonTest::testEmptyTransaction() {
    Swdb swdb(*conn);

    swdb.initTransaction();
    swdb.beginTransaction(1, "", "", 0);
    swdb.endTransaction(2, "", TransactionState::DONE);
    swdb.closeTransaction();

    CPPUNIT_ASSERT_EQUAL(
        static_cast<TransactionItemReason>(swdb.resolveRPMTransactionItemReason("bash", "x86_64", -1)),
        TransactionItemReason::UNKNOWN);
}

// one transaction with one transaction item -> $reason
void TransactionItemReasonTest::test_OneTransaction_OneTransactionItem() {
    Swdb swdb(*conn);

    swdb.initTransaction();

    auto rpm_bash = std::make_shared<Package>(*swdb.get_transaction_in_progress());
    rpm_bash->set_name("bash");
    rpm_bash->set_epoch("0");
    rpm_bash->set_version("4.4.12");
    rpm_bash->set_release("5.fc26");
    rpm_bash->set_arch("x86_64");
    std::string repoid = "base";
    TransactionItemAction action = TransactionItemAction::INSTALL;
    TransactionItemReason reason = TransactionItemReason::GROUP;
    auto ti = swdb.addItem(rpm_bash, repoid, action, reason);
    ti->set_state(TransactionItemState::DONE);

    swdb.beginTransaction(1, "", "", 0);
    swdb.endTransaction(2, "", TransactionState::DONE);
    swdb.closeTransaction();

    // package exists -> $reason
    CPPUNIT_ASSERT_EQUAL(
        static_cast<TransactionItemReason>(swdb.resolveRPMTransactionItemReason("bash", "x86_64", -1)),
        TransactionItemReason::GROUP);

    // package does not exist -> UNKNOWN
    CPPUNIT_ASSERT_EQUAL(
        static_cast<TransactionItemReason>(swdb.resolveRPMTransactionItemReason("bash", "i686", -1)),
        TransactionItemReason::UNKNOWN);

    // package exists, arch not specified -> return best $reason
    CPPUNIT_ASSERT_EQUAL(
        static_cast<TransactionItemReason>(swdb.resolveRPMTransactionItemReason("bash", "", -1)),
        TransactionItemReason::GROUP);
}

// one FAILED transaction with one transaction item -> $reason
void TransactionItemReasonTest::test_OneFailedTransaction_OneTransactionItem() {
    Swdb swdb(*conn);

    swdb.initTransaction();

    auto rpm_bash = std::make_shared<Package>(*swdb.get_transaction_in_progress());
    rpm_bash->set_name("bash");
    rpm_bash->set_epoch("0");
    rpm_bash->set_version("4.4.12");
    rpm_bash->set_release("5.fc26");
    rpm_bash->set_arch("x86_64");
    std::string repoid = "base";
    TransactionItemAction action = TransactionItemAction::INSTALL;
    TransactionItemReason reason = TransactionItemReason::GROUP;
    auto ti = swdb.addItem(rpm_bash, repoid, action, reason);
    ti->set_state(TransactionItemState::DONE);

    swdb.beginTransaction(1, "", "", 0);
    swdb.endTransaction(2, "", TransactionState::ERROR);
    swdb.closeTransaction();

    // failed transaction -> UNKNOWN
    CPPUNIT_ASSERT_EQUAL(
        static_cast<TransactionItemReason>(swdb.resolveRPMTransactionItemReason("bash", "x86_64", -1)),
        TransactionItemReason::UNKNOWN);

    // failed transaction -> UNKNOWN
    CPPUNIT_ASSERT_EQUAL(
        static_cast<TransactionItemReason>(swdb.resolveRPMTransactionItemReason("bash", "i686", -1)),
        TransactionItemReason::UNKNOWN);

    // failed transaction -> UNKNOWN
    CPPUNIT_ASSERT_EQUAL(
        static_cast<TransactionItemReason>(swdb.resolveRPMTransactionItemReason("bash", "", -1)),
        TransactionItemReason::UNKNOWN);
}

// one transaction with two transaction items -> $reason
void TransactionItemReasonTest::test_OneTransaction_TwoTransactionItems() {
    Swdb swdb(*conn);

    swdb.initTransaction();

    {
        auto rpm_bash = std::make_shared<Package>(*swdb.get_transaction_in_progress());
        rpm_bash->set_name("bash");
        rpm_bash->set_epoch("0");
        rpm_bash->set_version("4.4.12");
        rpm_bash->set_release("5.fc26");
        rpm_bash->set_arch("x86_64");
        std::string repoid = "base";
        TransactionItemAction action = TransactionItemAction::INSTALL;
        TransactionItemReason reason = TransactionItemReason::GROUP;
        auto ti = swdb.addItem(rpm_bash, repoid, action, reason);
        ti->set_state(TransactionItemState::DONE);
    }

    {
        auto rpm_bash = std::make_shared<Package>(*swdb.get_transaction_in_progress());
        rpm_bash->set_name("bash");
        rpm_bash->set_epoch("0");
        rpm_bash->set_version("4.4.12");
        rpm_bash->set_release("5.fc26");
        rpm_bash->set_arch("i686");
        std::string repoid = "base";
        TransactionItemAction action = TransactionItemAction::INSTALL;
        TransactionItemReason reason = TransactionItemReason::USER;
        auto ti = swdb.addItem(rpm_bash, repoid, action, reason);
        ti->set_state(TransactionItemState::DONE);
    }

    swdb.beginTransaction(1, "", "", 0);
    swdb.endTransaction(2, "", TransactionState::DONE);
    swdb.closeTransaction();

    // package exists -> $reason
    CPPUNIT_ASSERT_EQUAL(
        static_cast<TransactionItemReason>(swdb.resolveRPMTransactionItemReason("bash", "x86_64", -1)),
        TransactionItemReason::GROUP);

    // package exists -> $reason
    CPPUNIT_ASSERT_EQUAL(
        static_cast<TransactionItemReason>(swdb.resolveRPMTransactionItemReason("bash", "i686", -1)),
        TransactionItemReason::USER);

    // 2 packages exists, arch not specified -> return best $reason
    CPPUNIT_ASSERT_EQUAL(
        static_cast<TransactionItemReason>(swdb.resolveRPMTransactionItemReason("bash", "", -1)),
        TransactionItemReason::USER);
}

// two transactions with two transaction items -> $reason
void TransactionItemReasonTest::test_TwoTransactions_TwoTransactionItems() {
    Swdb swdb(*conn);

    {
        swdb.initTransaction();

        auto rpm_bash = std::make_shared<Package>(*swdb.get_transaction_in_progress());
        rpm_bash->set_name("bash");
        rpm_bash->set_epoch("0");
        rpm_bash->set_version("4.4.12");
        rpm_bash->set_release("5.fc26");
        rpm_bash->set_arch("x86_64");
        std::string repoid = "base";
        TransactionItemAction action = TransactionItemAction::INSTALL;
        TransactionItemReason reason = TransactionItemReason::GROUP;
        auto ti = swdb.addItem(rpm_bash, repoid, action, reason);
        ti->set_state(TransactionItemState::DONE);

        swdb.beginTransaction(1, "", "", 0);
        swdb.endTransaction(2, "", TransactionState::DONE);
        swdb.closeTransaction();
    }

    {
        swdb.initTransaction();

        auto rpm_bash = std::make_shared<Package>(*swdb.get_transaction_in_progress());
        rpm_bash->set_name("bash");
        rpm_bash->set_epoch("0");
        rpm_bash->set_version("4.4.12");
        rpm_bash->set_release("5.fc26");
        rpm_bash->set_arch("i686");
        std::string repoid = "base";
        TransactionItemAction action = TransactionItemAction::INSTALL;
        TransactionItemReason reason = TransactionItemReason::USER;
        auto ti = swdb.addItem(rpm_bash, repoid, action, reason);
        ti->set_state(TransactionItemState::DONE);

        swdb.beginTransaction(1, "", "", 0);
        swdb.endTransaction(2, "", TransactionState::DONE);
        swdb.closeTransaction();
    }

    // package exists -> $reason
    CPPUNIT_ASSERT_EQUAL(
        static_cast<TransactionItemReason>(swdb.resolveRPMTransactionItemReason("bash", "x86_64", -1)),
        TransactionItemReason::GROUP);

    // package exists -> $reason
    CPPUNIT_ASSERT_EQUAL(
        static_cast<TransactionItemReason>(swdb.resolveRPMTransactionItemReason("bash", "i686", -1)),
        TransactionItemReason::USER);

    // 2 packages exists, arch not specified -> return best $reason
    CPPUNIT_ASSERT_EQUAL(
        static_cast<TransactionItemReason>(swdb.resolveRPMTransactionItemReason("bash", "", -1)),
        TransactionItemReason::USER);
}

//
void TransactionItemReasonTest::testRemovedPackage() {
    Swdb swdb(*conn);

    {
        swdb.initTransaction();

        auto rpm_bash = std::make_shared<Package>(*swdb.get_transaction_in_progress());
        rpm_bash->set_name("bash");
        rpm_bash->set_epoch("0");
        rpm_bash->set_version("4.4.12");
        rpm_bash->set_release("5.fc26");
        rpm_bash->set_arch("x86_64");
        std::string repoid = "base";
        TransactionItemAction action = TransactionItemAction::INSTALL;
        TransactionItemReason reason = TransactionItemReason::GROUP;
        auto ti = swdb.addItem(rpm_bash, repoid, action, reason);
        ti->set_state(TransactionItemState::DONE);

        swdb.beginTransaction(1, "", "", 0);
        swdb.endTransaction(2, "", TransactionState::DONE);
        swdb.closeTransaction();
    }

    {
        swdb.initTransaction();

        auto rpm_bash = std::make_shared<Package>(*swdb.get_transaction_in_progress());
        rpm_bash->set_name("bash");
        rpm_bash->set_epoch("0");
        rpm_bash->set_version("4.4.12");
        rpm_bash->set_release("5.fc26");
        rpm_bash->set_arch("i686");
        std::string repoid = "base";
        TransactionItemAction action = TransactionItemAction::INSTALL;
        TransactionItemReason reason = TransactionItemReason::USER;
        auto ti = swdb.addItem(rpm_bash, repoid, action, reason);
        ti->set_state(TransactionItemState::DONE);

        swdb.beginTransaction(1, "", "", 0);
        swdb.endTransaction(2, "", TransactionState::DONE);
        swdb.closeTransaction();

        swdb.initTransaction();
        action = TransactionItemAction::REMOVE;
        auto ti_remove = swdb.addItem(rpm_bash, repoid, action, reason);
        ti_remove->set_state(TransactionItemState::DONE);
        swdb.beginTransaction(1, "", "", 0);
        swdb.endTransaction(2, "", TransactionState::DONE);
        swdb.closeTransaction();
    }

    // package exists -> $reason
    CPPUNIT_ASSERT_EQUAL(
        TransactionItemReason::GROUP,
        static_cast<TransactionItemReason>(swdb.resolveRPMTransactionItemReason("bash", "x86_64", -1)));

    // package exists -> $reason
    CPPUNIT_ASSERT_EQUAL(
        TransactionItemReason::UNKNOWN,
        static_cast<TransactionItemReason>(swdb.resolveRPMTransactionItemReason("bash", "i686", -1)));

    // 2 packages exists, arch not specified -> return best $reason
    CPPUNIT_ASSERT_EQUAL(
        TransactionItemReason::GROUP,
        static_cast<TransactionItemReason>(swdb.resolveRPMTransactionItemReason("bash", "", -1)));
}

void TransactionItemReasonTest::testCompareReasons() {
    CPPUNIT_ASSERT(TransactionItemReason::USER == TransactionItemReason::USER);
    CPPUNIT_ASSERT(TransactionItemReason::USER <= TransactionItemReason::USER);
    CPPUNIT_ASSERT(TransactionItemReason::USER >= TransactionItemReason::USER);

    CPPUNIT_ASSERT(TransactionItemReason::USER != TransactionItemReason::GROUP);
    CPPUNIT_ASSERT(TransactionItemReason::USER > TransactionItemReason::GROUP);
    CPPUNIT_ASSERT(TransactionItemReason::USER >= TransactionItemReason::GROUP);

    CPPUNIT_ASSERT(TransactionItemReason::GROUP != TransactionItemReason::USER);
    CPPUNIT_ASSERT(TransactionItemReason::GROUP < TransactionItemReason::USER);
    CPPUNIT_ASSERT(TransactionItemReason::GROUP <= TransactionItemReason::USER);
}

void TransactionItemReasonTest::test_TransactionItemReason_compare() {
    CPPUNIT_ASSERT_EQUAL(-1, TransactionItemReason_compare(TransactionItemReason::GROUP, TransactionItemReason::USER));
    CPPUNIT_ASSERT_EQUAL(0, TransactionItemReason_compare(TransactionItemReason::USER, TransactionItemReason::USER));
    CPPUNIT_ASSERT_EQUAL(1, TransactionItemReason_compare(TransactionItemReason::USER, TransactionItemReason::GROUP));
}

#endif
