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


#ifndef TEST_LIBDNF_ADVISORY_ADVISORY_PACKAGE_HPP
#define TEST_LIBDNF_ADVISORY_ADVISORY_PACKAGE_HPP


#include "base_test_case.hpp"

#include "libdnf/advisory/advisory_collection.hpp"
#include "libdnf/advisory/advisory_package.hpp"

#include <cppunit/extensions/HelperMacros.h>

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
    std::vector<libdnf::advisory::AdvisoryPackage> packages;
};


#endif  // TEST_LIBDNF_ADVISORY_ADVISORY_PACKAGE_HPP
