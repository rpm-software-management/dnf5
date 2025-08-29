// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_CONF_CONF_HPP
#define TEST_LIBDNF5_CONF_CONF_HPP

#include "../shared/test_case_fixture.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/base/base.hpp>
#include <libdnf5/logger/log_router.hpp>


class ConfTest : public TestCaseFixture {
    CPPUNIT_TEST_SUITE(ConfTest);
    CPPUNIT_TEST(test_config_main);
    CPPUNIT_TEST(test_config_repo);
    CPPUNIT_TEST(test_config_pkg_gpgcheck);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_config_main();
    void test_config_repo();
    void test_config_pkg_gpgcheck();

    std::unique_ptr<libdnf5::Base> base;
    libdnf5::LogRouter logger;
    libdnf5::ConfigMain config;
};


#endif
