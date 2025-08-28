// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_BASE_GOAL_HPP
#define TEST_LIBDNF5_BASE_GOAL_HPP


#include "libdnf_private_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>


class BaseGoalTest : public LibdnfPrivateTestCase {
    CPPUNIT_TEST_SUITE(BaseGoalTest);

#ifndef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_install);
    CPPUNIT_TEST(test_install_not_available);
    CPPUNIT_TEST(test_install_multilib_all);
    CPPUNIT_TEST(test_install_installed_pkg);
    CPPUNIT_TEST(test_install_or_reinstall);
    CPPUNIT_TEST(test_install_from_cmdline);
    CPPUNIT_TEST(test_reinstall);
    CPPUNIT_TEST(test_reinstall_from_cmdline);
    CPPUNIT_TEST(test_reinstall_user);
    CPPUNIT_TEST(test_remove);
    CPPUNIT_TEST(test_remove_not_installed);
    CPPUNIT_TEST(test_upgrade);
    CPPUNIT_TEST(test_upgrade_from_cmdline);
    CPPUNIT_TEST(test_upgrade_not_downgrade_from_cmdline);
    CPPUNIT_TEST(test_upgrade_not_available);
    CPPUNIT_TEST(test_upgrade_all);
    CPPUNIT_TEST(test_upgrade_user);
    CPPUNIT_TEST(test_downgrade);
    CPPUNIT_TEST(test_downgrade_from_cmdline);
    CPPUNIT_TEST(test_downgrade_user);
    CPPUNIT_TEST(test_distrosync);
    CPPUNIT_TEST(test_distrosync_all);
#endif

#ifdef WITH_PERFORMANCE_TESTS
#endif

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_install();
    void test_install_not_available();
    void test_install_multilib_all();
    void test_install_installed_pkg();
    void test_install_or_reinstall();
    void test_install_from_cmdline();
    void test_reinstall();
    void test_reinstall_from_cmdline();
    void test_reinstall_user();
    void test_remove();
    void test_remove_not_installed();
    void test_upgrade();
    void test_upgrade_from_cmdline();
    void test_upgrade_not_downgrade_from_cmdline();
    void test_upgrade_not_available();
    void test_upgrade_all();
    void test_upgrade_user();
    void test_downgrade();
    void test_downgrade_from_cmdline();
    void test_downgrade_user();
    void test_distrosync();
    void test_distrosync_all();
};


#endif  // TEST_LIBDNF5_BASE_GOAL_HPP
