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


#ifndef TEST_LIBDNF5_RPM_PACKAGE_QUERY_HPP
#define TEST_LIBDNF5_RPM_PACKAGE_QUERY_HPP


#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>


class RpmPackageQueryTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(RpmPackageQueryTest);

#ifndef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_size);
    CPPUNIT_TEST(test_filter_latest_evr);
    CPPUNIT_TEST(test_filter_latest_evr_ignore_arch);
    CPPUNIT_TEST(test_filter_earliest_evr);
    CPPUNIT_TEST(test_filter_earliest_evr_ignore_arch);
    CPPUNIT_TEST(test_filter_name);
    CPPUNIT_TEST(test_filter_name_packgset);
    CPPUNIT_TEST(test_filter_nevra_packgset);
    CPPUNIT_TEST(test_filter_nevra_packgset_cmp);
    CPPUNIT_TEST(test_filter_name_arch);
    CPPUNIT_TEST(test_filter_name_arch2);
    CPPUNIT_TEST(test_filter_nevra);
    CPPUNIT_TEST(test_filter_version);
    CPPUNIT_TEST(test_filter_release);
    CPPUNIT_TEST(test_filter_priority);
    CPPUNIT_TEST(test_filter_provides);
    CPPUNIT_TEST(test_filter_requires);
    CPPUNIT_TEST(test_filter_advisories);
    CPPUNIT_TEST(test_filter_chain);
    CPPUNIT_TEST(test_resolve_pkg_spec);
    CPPUNIT_TEST(test_update);
    CPPUNIT_TEST(test_intersection);
    CPPUNIT_TEST(test_difference);
#endif

#ifdef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_filter_latest_evr_performance);
    CPPUNIT_TEST(test_filter_provides_performance);
#endif

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_size();
    void test_filter_latest_evr();
    void test_filter_latest_evr_ignore_arch();
    void test_filter_earliest_evr();
    void test_filter_earliest_evr_ignore_arch();
    void test_filter_name();
    void test_filter_name_packgset();
    void test_filter_nevra_packgset();
    void test_filter_nevra_packgset_cmp();
    void test_filter_name_arch();
    void test_filter_name_arch2();
    void test_filter_nevra();
    void test_filter_version();
    void test_filter_release();
    void test_filter_provides();
    void test_filter_priority();
    void test_filter_requires();
    void test_filter_advisories();
    void test_filter_chain();
    void test_resolve_pkg_spec();
    void test_update();
    void test_intersection();
    void test_difference();

    void test_filter_latest_evr_performance();
    void test_filter_provides_performance();

    // TODO(jmracek) Add tests when system repo will be available
    // PackageQuery & filter_upgrades();
    // PackageQuery & filter_downgrades();
    // PackageQuery & filter_upgradable();
    // PackageQuery & filter_downgradable();
};


#endif  // TEST_LIBDNF5_RPM_PACKAGE_QUERY_HPP
