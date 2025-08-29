// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifdef WITH_MODULEMD


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
    CPPUNIT_TEST(test_module_enable_default);
    CPPUNIT_TEST(test_module_disable);
    CPPUNIT_TEST(test_module_disable_enabled);
    CPPUNIT_TEST(test_module_reset);
    CPPUNIT_TEST(test_module_globs);
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
    void test_module_enable_default();
    void test_module_disable();
    void test_module_disable_enabled();
    void test_module_reset();
    void test_module_globs();

    std::unique_ptr<libdnf5::utils::fs::TempDir> temp_dir;
};

#endif

#endif  // WITH_MODULEMD
