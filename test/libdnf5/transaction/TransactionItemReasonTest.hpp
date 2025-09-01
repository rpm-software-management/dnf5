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

#ifndef LIBDNF5_SWDB_TRANSACTION_ITEM_REASON_TEST_HPP
#define LIBDNF5_SWDB_TRANSACTION_ITEM_REASON_TEST_HPP

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/utils/sqlite3/sqlite3.hpp>

class TransactionItemReasonTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(TransactionItemReasonTest);
    CPPUNIT_TEST(testNoTransaction);
    CPPUNIT_TEST(testEmptyTransaction);
    CPPUNIT_TEST(test_OneTransaction_OneTransactionItem);
    CPPUNIT_TEST(test_OneFailedTransaction_OneTransactionItem);
    CPPUNIT_TEST(test_OneTransaction_TwoTransactionItems);
    CPPUNIT_TEST(test_TwoTransactions_TwoTransactionItems);
    CPPUNIT_TEST(testRemovedPackage);
    CPPUNIT_TEST(testCompareReasons);
    CPPUNIT_TEST(test_TransactionItemReason_compare);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void testNoTransaction();
    void testEmptyTransaction();
    void test_OneTransaction_OneTransactionItem();
    void test_OneFailedTransaction_OneTransactionItem();
    void test_OneTransaction_TwoTransactionItems();
    void test_TwoTransactions_TwoTransactionItems();
    void testRemovedPackage();
    void testCompareReasons();
    void test_TransactionItemReason_compare();

private:
    libdnf5::utils::SQLite3 * conn;
};

#endif  // LIBDNF5_SWDB_TRANSACTION_ITEM_REASON_TEST_HPP

#endif
