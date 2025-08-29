// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_CLI_ARGUMENT_PARSER_HPP
#define TEST_LIBDNF5_CLI_ARGUMENT_PARSER_HPP

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

class ArgumentParserTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(ArgumentParserTest);

    CPPUNIT_TEST(test_argument_parser);

    CPPUNIT_TEST_SUITE_END();

public:
    void test_argument_parser();
};


#endif  // TEST_LIBDNF5_CLI_ARGUMENT_PARSER_HPP
