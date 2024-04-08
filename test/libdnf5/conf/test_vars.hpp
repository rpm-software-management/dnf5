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


#ifndef TEST_LIBDNF5_CONF_VARS_HPP
#define TEST_LIBDNF5_CONF_VARS_HPP

#include "../shared/test_case_fixture.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/base/base.hpp>


class VarsTest : public TestCaseFixture {
    CPPUNIT_TEST_SUITE(VarsTest);
    CPPUNIT_TEST(test_vars);
    CPPUNIT_TEST(test_vars_multiple_dirs);
    CPPUNIT_TEST(test_vars_env);
    CPPUNIT_TEST(test_vars_api);
    CPPUNIT_TEST(test_vars_api_set_prio);
    CPPUNIT_TEST(test_vars_api_unset_prio);
    CPPUNIT_TEST(test_vars_api_releasever);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_vars();
    void test_vars_multiple_dirs();
    void test_vars_env();
    void test_vars_api();
    void test_vars_api_set_prio();
    void test_vars_api_unset_prio();
    void test_vars_api_releasever();

    std::unique_ptr<libdnf5::Base> base;
};


#endif
