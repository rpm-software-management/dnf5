// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_CONFIG_PARSER_HPP
#define LIBDNF5_TEST_CONFIG_PARSER_HPP

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class ConfigParserTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(ConfigParserTest);

#ifndef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_create_simple);
    CPPUNIT_TEST(test_create_with_comments_header);
    CPPUNIT_TEST(test_create_crazy);
    CPPUNIT_TEST(test_parse_check_results_simple);
    CPPUNIT_TEST(test_parse_check_results_with_comments_header);
    CPPUNIT_TEST(test_parse_check_results_crazy);
    CPPUNIT_TEST(test_read_write_simple);
    CPPUNIT_TEST(test_read_write_with_comments_header);
    CPPUNIT_TEST(test_read_write_crazy);
    CPPUNIT_TEST(test_read_modify_write);
#endif

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_create_simple();
    void test_create_with_comments_header();
    void test_create_crazy();
    void test_parse_check_results_simple();
    void test_parse_check_results_with_comments_header();
    void test_parse_check_results_crazy();
    void test_read_write_simple();
    void test_read_write_with_comments_header();
    void test_read_write_crazy();
    void test_read_modify_write();
};

#endif
