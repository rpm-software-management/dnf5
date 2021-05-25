/*
Copyright (C) 2021 Red Hat, Inc.

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


#ifndef TEST_LIBDNF_ADVISORY_ADVISORY_QUERY_HPP
#define TEST_LIBDNF_ADVISORY_ADVISORY_QUERY_HPP


#include "../rpm/repo_fixture.hpp"

#include "libdnf/advisory/advisory_sack.hpp"

#include <cppunit/extensions/HelperMacros.h>

class AdvisoryAdvisoryQueryTest : public RepoFixture {
    CPPUNIT_TEST_SUITE(AdvisoryAdvisoryQueryTest);

    CPPUNIT_TEST(test_size);
    CPPUNIT_TEST(test_filter_name);
    CPPUNIT_TEST(test_filter_type);
    CPPUNIT_TEST(test_filter_packages);
    CPPUNIT_TEST(test_filter_cve);
    CPPUNIT_TEST(test_filter_bug);
    CPPUNIT_TEST(test_filter_severity);
    CPPUNIT_TEST(test_get_sorted_advisory_packages);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_size();
    void test_filter_name();
    void test_filter_type();
    void test_filter_packages();
    void test_filter_cve();
    void test_filter_bug();
    void test_filter_severity();
    void test_get_sorted_advisory_packages();

private:
    libdnf::advisory::AdvisorySackWeakPtr advisory_sack;
};


#endif  // TEST_LIBDNF_ADVISORY_ADVISORY_QUERY_HPP
