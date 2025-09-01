// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#ifndef TEST_LIBDNF5_ADVISORY_ADVISORY_MODULE_HPP
#define TEST_LIBDNF5_ADVISORY_ADVISORY_MODULE_HPP


#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/advisory/advisory_collection.hpp>
#include <libdnf5/advisory/advisory_module.hpp>

class AdvisoryAdvisoryModuleTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(AdvisoryAdvisoryModuleTest);

    CPPUNIT_TEST(test_get_name);
    CPPUNIT_TEST(test_get_stream);
    CPPUNIT_TEST(test_get_version);
    CPPUNIT_TEST(test_get_context);
    CPPUNIT_TEST(test_get_arch);

    CPPUNIT_TEST(test_get_advisory_id);
    CPPUNIT_TEST(test_get_advisory);
    CPPUNIT_TEST(test_get_advisory_collection);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_get_name();
    void test_get_stream();
    void test_get_version();
    void test_get_context();
    void test_get_arch();

    void test_get_advisory_id();
    void test_get_advisory();
    void test_get_advisory_collection();

private:
    std::vector<libdnf5::advisory::AdvisoryModule> modules;
};


#endif  // TEST_LIBDNF5_ADVISORY_ADVISORY_MODULE_HPP
