// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_INIPARSER_HPP
#define LIBDNF5_TEST_INIPARSER_HPP

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class IniparserTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(IniparserTest);

#ifndef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_iniparser);
    CPPUNIT_TEST(test_iniparser2);
    CPPUNIT_TEST(test_iniparser_missing_section_header);
    CPPUNIT_TEST(test_iniparser_missing_bracket);
    CPPUNIT_TEST(test_iniparser_missing_bracket2);
    CPPUNIT_TEST(test_iniparser_empty_section_name);
    CPPUNIT_TEST(test_iniparser_text_after_section);
    CPPUNIT_TEST(test_iniparser_illegal_continuation_line);
    CPPUNIT_TEST(test_iniparser_missing_key);
    CPPUNIT_TEST(test_iniparser_missing_equal);
#endif

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_iniparser();
    void test_iniparser2();
    void test_iniparser_missing_section_header();
    void test_iniparser_missing_bracket();
    void test_iniparser_missing_bracket2();
    void test_iniparser_empty_section_name();
    void test_iniparser_text_after_section();
    void test_iniparser_illegal_continuation_line();
    void test_iniparser_missing_key();
    void test_iniparser_missing_equal();
};

#endif
