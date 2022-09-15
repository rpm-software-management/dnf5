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


#ifndef TEST_LIBDNF_BASE_BASE_HPP
#define TEST_LIBDNF_BASE_BASE_HPP


#include "test_case_fixture.hpp"

#include <cppunit/extensions/HelperMacros.h>


class BaseTest : public TestCaseFixture {
    CPPUNIT_TEST_SUITE(BaseTest);
    CPPUNIT_TEST(test_weak_ptr);
    CPPUNIT_TEST(test_incorrect_workflow);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_weak_ptr();
    void test_incorrect_workflow();
};


#endif  // TEST_LIBDNF_BASE_GOAL_HPP
