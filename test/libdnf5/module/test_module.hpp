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


#ifndef LIBDNF5_TEST_MODULE_HPP
#define LIBDNF5_TEST_MODULE_HPP

#include "../shared/base_test_case.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class ModuleTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(ModuleTest);
    CPPUNIT_TEST(test_load);
    CPPUNIT_TEST(test_resolve);
    CPPUNIT_TEST(test_resolve_broken_defaults);
    CPPUNIT_TEST(test_query);
    CPPUNIT_TEST(test_query_latest);
    CPPUNIT_TEST(test_nsvcap);
    CPPUNIT_TEST(test_query_spec);
    CPPUNIT_TEST(test_module_db);
    CPPUNIT_TEST(test_module_enable);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_load();
    void test_resolve();
    void test_resolve_broken_defaults();
    void test_query();
    void test_query_latest();
    void test_nsvcap();
    void test_query_spec();
    void test_module_db();
    void test_module_enable();

    std::unique_ptr<libdnf5::utils::fs::TempDir> temp_dir;
};

#endif
