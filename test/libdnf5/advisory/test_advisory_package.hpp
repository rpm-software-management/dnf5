// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_ADVISORY_ADVISORY_PACKAGE_HPP
#define TEST_LIBDNF5_ADVISORY_ADVISORY_PACKAGE_HPP


#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/advisory/advisory_collection.hpp>
#include <libdnf5/advisory/advisory_package.hpp>

class AdvisoryAdvisoryPackageTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(AdvisoryAdvisoryPackageTest);

    CPPUNIT_TEST(test_get_name);
    CPPUNIT_TEST(test_get_version);
    CPPUNIT_TEST(test_get_evr);
    CPPUNIT_TEST(test_get_arch);
    CPPUNIT_TEST(test_get_advisory_id);
    CPPUNIT_TEST(test_get_advisory);
    CPPUNIT_TEST(test_get_advisory_collection);
    CPPUNIT_TEST(test_get_reboot_suggested);
    CPPUNIT_TEST(test_get_restart_suggested);
    CPPUNIT_TEST(test_get_relogin_suggested);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_get_name();
    void test_get_version();
    void test_get_evr();
    void test_get_arch();
    void test_get_advisory_id();
    void test_get_advisory();
    void test_get_advisory_collection();
    void test_get_reboot_suggested();
    void test_get_restart_suggested();
    void test_get_relogin_suggested();

private:
    std::vector<libdnf5::advisory::AdvisoryPackage> packages;
};


#endif  // TEST_LIBDNF5_ADVISORY_ADVISORY_PACKAGE_HPP
