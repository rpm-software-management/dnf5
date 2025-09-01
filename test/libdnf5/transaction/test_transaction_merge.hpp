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


#ifndef TEST_LIBDNF5_TRANSACTION_TEST_TRANSACTION_MERGE_HPP
#define TEST_LIBDNF5_TRANSACTION_TEST_TRANSACTION_MERGE_HPP


#include "transaction_test_base.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class TransactionMergeTest : public TransactionTestBase {
    CPPUNIT_TEST_SUITE(TransactionMergeTest);
    CPPUNIT_TEST(empty_transaction);
    CPPUNIT_TEST(only_one_transaction);
    CPPUNIT_TEST(two_disjoint);

    CPPUNIT_TEST(install_install);
    CPPUNIT_TEST(install_install2);
    CPPUNIT_TEST(install_upgrade);
    CPPUNIT_TEST(install_downgrade);
    CPPUNIT_TEST(install_reinstall);
    CPPUNIT_TEST(install_remove);
    CPPUNIT_TEST(install_replaced);
    CPPUNIT_TEST(install_reasonchange);

    CPPUNIT_TEST(upgrade_install);
    CPPUNIT_TEST(upgrade_upgrade);
    CPPUNIT_TEST(upgrade_upgrade2);
    CPPUNIT_TEST(upgrade_upgrade_installonly);
    CPPUNIT_TEST(upgrade_downgrade);
    CPPUNIT_TEST(upgrade_reinstall);
    CPPUNIT_TEST(upgrade_remove);
    CPPUNIT_TEST(upgrade_replaced);
    CPPUNIT_TEST(upgrade_reasonchange);
    CPPUNIT_TEST(upgrade_missing);

    CPPUNIT_TEST(remove_install);
    CPPUNIT_TEST(remove_upgrade);
    CPPUNIT_TEST(remove_downgrade);
    CPPUNIT_TEST(remove_reinstall);
    CPPUNIT_TEST(remove_remove);
    CPPUNIT_TEST(remove_remove_installonly);
    CPPUNIT_TEST(remove_replaced);
    CPPUNIT_TEST(remove_reasonchange);
    CPPUNIT_TEST(remove_missing);

    CPPUNIT_TEST(reinstall);
    CPPUNIT_TEST(reinstall_install);
    CPPUNIT_TEST(reinstall_install2);
    CPPUNIT_TEST(reinstall_upgrade);
    CPPUNIT_TEST(reinstall_downgrade);
    CPPUNIT_TEST(reinstall_reinstall);
    CPPUNIT_TEST(reinstall_remove);
    CPPUNIT_TEST(reinstall_replaced);
    CPPUNIT_TEST(reinstall_reasonchange);
    CPPUNIT_TEST(reinstall_missing);
    CPPUNIT_TEST(reinstall_wrong_version);

    CPPUNIT_TEST(reasonchange);
    CPPUNIT_TEST(reasonchange_install);
    CPPUNIT_TEST(reasonchange_missing);
    CPPUNIT_TEST(reasonchange_upgrade);
    CPPUNIT_TEST(reasonchange_downgrade);
    CPPUNIT_TEST(reasonchange_reinstall);
    CPPUNIT_TEST(reasonchange_remove);
    CPPUNIT_TEST(reasonchange_replaced);
    CPPUNIT_TEST(reasonchange_reasonchange);

    CPPUNIT_TEST(install_upgrade_reinstall);
    CPPUNIT_TEST(install_remove_install);
    CPPUNIT_TEST(install_remove_upgrade);
    CPPUNIT_TEST(install_install_remove);
    CPPUNIT_TEST(install_install_installonly);
    CPPUNIT_TEST(install_remove_installonly);
    CPPUNIT_TEST(remove_installonly_missing);
    CPPUNIT_TEST(install_remove_install_installonly);
    CPPUNIT_TEST(remove_install_remove);
    CPPUNIT_TEST(install_reasonchange_upgrade);
    CPPUNIT_TEST(remove_install_lower);
    CPPUNIT_TEST(upgrade_installonly_forward);
    CPPUNIT_TEST(upgrade_installonly_backward);

    CPPUNIT_TEST(group_install_remove);
    CPPUNIT_TEST(group_install_install);
    CPPUNIT_TEST(group_install_upgrade);
    CPPUNIT_TEST(group_remove_install);
    CPPUNIT_TEST(group_remove_upgrade);
    CPPUNIT_TEST(group_remove_remove);
    CPPUNIT_TEST(group_upgrade_upgrade);
    CPPUNIT_TEST(group_upgrade_remove);
    CPPUNIT_TEST(group_upgrade_install);

    CPPUNIT_TEST_SUITE_END();

public:
    void empty_transaction();
    void only_one_transaction();
    void two_disjoint();

    void install_install();
    void install_install2();
    void install_upgrade();
    void install_downgrade();
    void install_reinstall();
    void install_remove();
    void install_replaced();
    void install_reasonchange();

    void upgrade_install();
    void upgrade_upgrade();
    void upgrade_upgrade2();
    void upgrade_upgrade_installonly();
    void upgrade_downgrade();
    void upgrade_reinstall();
    void upgrade_remove();
    void upgrade_replaced();
    void upgrade_reasonchange();
    void upgrade_missing();

    void reinstall();
    void reinstall_install();
    void reinstall_install2();
    void reinstall_upgrade();
    void reinstall_downgrade();
    void reinstall_reinstall();
    void reinstall_remove();
    void reinstall_replaced();
    void reinstall_reasonchange();
    void reinstall_missing();
    void reinstall_wrong_version();

    void remove_install();
    void remove_upgrade();
    void remove_downgrade();
    void remove_reinstall();
    void remove_remove();
    void remove_remove_installonly();
    void remove_replaced();
    void remove_reasonchange();
    void remove_missing();

    void reasonchange();
    void reasonchange_install();
    void reasonchange_missing();
    void reasonchange_upgrade();
    void reasonchange_downgrade();
    void reasonchange_reinstall();
    void reasonchange_remove();
    void reasonchange_replaced();
    void reasonchange_reasonchange();

    void install_upgrade_reinstall();
    void install_remove_install();
    void install_remove_upgrade();
    void install_install_remove();
    void install_install_installonly();
    void install_remove_installonly();
    void remove_installonly_missing();
    void install_remove_install_installonly();
    void remove_install_lower();
    void remove_install_remove();
    void install_reasonchange_upgrade();
    void upgrade_installonly_forward();
    void upgrade_installonly_backward();

    void group_install_remove();
    void group_install_install();
    void group_install_upgrade();
    void group_remove_install();
    void group_remove_upgrade();
    void group_remove_remove();
    void group_upgrade_upgrade();
    void group_upgrade_remove();
    void group_upgrade_install();

    // Only a couple tests for environments, they share code with groups
    void env_install_remove();
    void env_upgrade_remove();
};


#endif  // TEST_LIBDNF5_TRANSACTION_TEST_TRANSACTION_MERGE_HPP
