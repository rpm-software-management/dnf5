// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_RPM_PACKAGE_SACK_HPP
#define TEST_LIBDNF5_RPM_PACKAGE_SACK_HPP


#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/rpm/package_set.hpp>


class RpmPackageSackTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(RpmPackageSackTest);

    CPPUNIT_TEST(test_set_user_excludes);
    CPPUNIT_TEST(test_add_user_excludes);
    CPPUNIT_TEST(test_remove_user_excludes);

    CPPUNIT_TEST(test_set_user_includes);
    CPPUNIT_TEST(test_add_user_includes);
    CPPUNIT_TEST(test_remove_user_includes);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_set_user_excludes();
    void test_add_user_excludes();
    void test_remove_user_excludes();

    void test_set_user_includes();
    void test_add_user_includes();
    void test_remove_user_includes();

private:
    std::unique_ptr<libdnf5::rpm::PackageSet> pkgset;
    std::unique_ptr<libdnf5::rpm::Package> pkg0;
};


#endif  // TEST_LIBDNF5_RPM_PACKAGE_SACK_HPP
