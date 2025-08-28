// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_CONF_OPTION_HPP
#define TEST_LIBDNF5_CONF_OPTION_HPP

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class OptionTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(OptionTest);
    CPPUNIT_TEST(test_options_bool);
    CPPUNIT_TEST(test_options_child);
    CPPUNIT_TEST(test_options_enum);
    CPPUNIT_TEST(test_options_number);
    CPPUNIT_TEST(test_options_path);
    CPPUNIT_TEST(test_options_seconds);
    CPPUNIT_TEST(test_options_string);
    CPPUNIT_TEST(test_options_string_list);
    CPPUNIT_TEST(test_options_string_list_delimiters);
    CPPUNIT_TEST(test_options_string_list_custom_delimiters);
    CPPUNIT_TEST(test_options_string_set);
    CPPUNIT_TEST(test_options_list_add);
    CPPUNIT_TEST(test_options_list_add_item);
    CPPUNIT_TEST(test_options_string_append_list);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_options_bool();
    void test_options_child();
    void test_options_enum();
    void test_options_number();
    void test_options_path();
    void test_options_seconds();
    void test_options_string();
    void test_options_string_list();
    void test_options_string_list_delimiters();
    void test_options_string_list_custom_delimiters();
    void test_options_string_set();
    void test_options_list_add();
    void test_options_list_add_item();
    void test_options_string_append_list();
};


#endif
