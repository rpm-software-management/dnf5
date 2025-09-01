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

#ifndef LIBDNF5_SWDB_MERGEDTRANSACTION_TEST_HPP
#define LIBDNF5_SWDB_MERGEDTRANSACTION_TEST_HPP

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/utils/sqlite3/sqlite3.hpp>

class MergedTransactionTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(MergedTransactionTest);
    //     CPPUNIT_TEST(testMerge);
    //     CPPUNIT_TEST(testMergeEraseInstallReinstall);
    //     CPPUNIT_TEST(testMergeEraseInstallDowngrade);
    //     CPPUNIT_TEST(testMergeEraseInstallUpgrade);
    //     CPPUNIT_TEST(testMergeReinstallAny);
    //     CPPUNIT_TEST(testMergeInstallErase);
    //     CPPUNIT_TEST(testMergeInstallAlter);
    //     CPPUNIT_TEST(testMergeAlterReinstall);
    //     CPPUNIT_TEST(testMergeAlterErase);
    //     CPPUNIT_TEST(testMergeAlterAlter);
    CPPUNIT_TEST(test_add_remove_installed);
    CPPUNIT_TEST(test_add_remove_removed);
    CPPUNIT_TEST(test_add_install_installed);
    CPPUNIT_TEST(test_add_install_removed);
    CPPUNIT_TEST(test_add_obsoleted_installed);
    CPPUNIT_TEST(test_add_obsoleted_obsoleted);

    CPPUNIT_TEST(test_downgrade);
    CPPUNIT_TEST(test_install_downgrade);

    CPPUNIT_TEST(test_multilib_identity);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void testMerge();
    void testMergeEraseInstallReinstall();
    void testMergeEraseInstallDowngrade();
    void testMergeEraseInstallUpgrade();
    void testMergeReinstallAny();
    void testMergeInstallErase();
    void testMergeInstallAlter();
    void testMergeAlterReinstall();
    void testMergeAlterErase();
    void testMergeAlterAlter();
    // BEGIN: tests ported from DNF unit tests
    void test_add_remove_installed();
    void test_add_remove_removed();
    void test_add_install_installed();
    void test_add_install_removed();
    void test_add_obsoleted_installed();
    void test_add_obsoleted_obsoleted();
    // END: tests ported from DNF unit tests

    void test_downgrade();
    void test_install_downgrade();

    void test_multilib_identity();

private:
    libdnf5::utils::SQLite3 * conn;
};

#endif  // LIBDNF5_SWDB_MERGEDTRANSACTION_TEST_HPP

#endif
