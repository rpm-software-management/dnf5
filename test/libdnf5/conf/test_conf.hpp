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
