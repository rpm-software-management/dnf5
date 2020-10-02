/*
Copyright (C) 2020 Red Hat, Inc.

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


#ifndef TEST_LIBDNF_RPM_RELDEP_LIST_HPP
#define TEST_LIBDNF_RPM_RELDEP_LIST_HPP

#include "repo_fixture.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/rpm/solv_sack.hpp"

#include <cppunit/extensions/HelperMacros.h>


class ReldepListTest : public RepoFixture {
    CPPUNIT_TEST_SUITE(ReldepListTest);
    CPPUNIT_TEST(test_get);
    CPPUNIT_TEST(test_add);
    CPPUNIT_TEST(test_size);
    CPPUNIT_TEST(test_compare);
    CPPUNIT_TEST(test_append);
    CPPUNIT_TEST(test_iterator);
    CPPUNIT_TEST(test_add_reldep_with_glob);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_get();
    void test_add();
    void test_size();
    void test_compare();
    void test_append();
    void test_iterator();
    void test_add_reldep_with_glob();
};

#endif  // TEST_LIBDNF_RPM_RELDEP_LIST_HPP
