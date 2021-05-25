/*
Copyright (C) 2020-2021 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef TEST_LIBDNF_RPM_PACKAGE_QUERY_HPP
#define TEST_LIBDNF_RPM_PACKAGE_QUERY_HPP


#include "repo_fixture.hpp"

#include <cppunit/extensions/HelperMacros.h>


class RpmPackageQueryTest : public RepoFixture {
    CPPUNIT_TEST_SUITE(RpmPackageQueryTest);

#ifndef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_size);
    CPPUNIT_TEST(test_filter_latest);
    CPPUNIT_TEST(test_filter_name);
    CPPUNIT_TEST(test_filter_name_packgset);
    CPPUNIT_TEST(test_filter_nevra_packgset);
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
#endif

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_size();
    void test_filter_latest();
    void test_filter_name();
    void test_filter_name_packgset();
    void test_filter_nevra_packgset();
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
    // TODO(jmracek) Add tests when system repo will be available
    // PackageQuery & filter_upgrades();
    // PackageQuery & filter_downgrades();
    // PackageQuery & filter_upgradable();
    // PackageQuery & filter_downgradable();
};


#endif  // TEST_LIBDNF_RPM_PACKAGE_QUERY_HPP
