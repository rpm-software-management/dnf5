// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


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
