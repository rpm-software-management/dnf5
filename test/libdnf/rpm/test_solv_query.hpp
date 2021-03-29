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


#ifndef TEST_LIBDNF_RPM_SOLV_QUERY_HPP
#define TEST_LIBDNF_RPM_SOLV_QUERY_HPP


#include "repo_fixture.hpp"

#include <cppunit/extensions/HelperMacros.h>


class RpmSolvQueryTest : public RepoFixture {
    CPPUNIT_TEST_SUITE(RpmSolvQueryTest);

#ifndef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_size);
    CPPUNIT_TEST(test_ifilter_latest);
    CPPUNIT_TEST(test_ifilter_name);
    CPPUNIT_TEST(test_ifilter_name_packgset);
    CPPUNIT_TEST(test_ifilter_name_arch);
    CPPUNIT_TEST(test_ifilter_nevra);
    CPPUNIT_TEST(test_ifilter_version);
    CPPUNIT_TEST(test_ifilter_release);
    CPPUNIT_TEST(test_ifilter_priority);
    CPPUNIT_TEST(test_ifilter_provides);
    CPPUNIT_TEST(test_ifilter_requires);
    CPPUNIT_TEST(test_ifilter_advisories);
    CPPUNIT_TEST(test_ifilter_chain);
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
    void test_ifilter_latest();
    void test_ifilter_name();
    void test_ifilter_name_packgset();
    void test_ifilter_name_arch();
    void test_ifilter_nevra();
    void test_ifilter_version();
    void test_ifilter_release();
    void test_ifilter_provides();
    void test_ifilter_priority();
    void test_ifilter_requires();
    void test_ifilter_advisories();
    void test_ifilter_chain();
    void test_resolve_pkg_spec();
    void test_update();
    void test_intersection();
    void test_difference();
    // TODO(jmracek) Add tests when system repo will be available
    // SolvQuery & ifilter_upgrades();
    // SolvQuery & ifilter_downgrades();
    // SolvQuery & ifilter_upgradable();
    // SolvQuery & ifilter_downgradable();
};


#endif  // TEST_LIBDNF_RPM_SOLV_QUERY_HPP
