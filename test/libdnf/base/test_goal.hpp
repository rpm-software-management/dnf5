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


#ifndef TEST_LIBDNF_BASE_GOAL_HPP
#define TEST_LIBDNF_BASE_GOAL_HPP


#include "../rpm/repo_fixture.hpp"

#include <cppunit/extensions/HelperMacros.h>


class BaseGoalTest : public RepoFixture {
    CPPUNIT_TEST_SUITE(BaseGoalTest);

#ifndef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_install);
    CPPUNIT_TEST(test_install_from_cmdline);
    CPPUNIT_TEST(test_remove);
#endif

#ifdef WITH_PERFORMANCE_TESTS
#endif

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_install();
    void test_install_from_cmdline();
    void test_remove();
};


#endif  // TEST_LIBDNF_BASE_GOAL_HPP
