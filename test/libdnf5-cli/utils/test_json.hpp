// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef TEST_LIBDNF5_CLI_UTILS_JSON_HPP
#define TEST_LIBDNF5_CLI_UTILS_JSON_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <vector>


class JSONTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(JSONTest);

    CPPUNIT_TEST(test_normalize_field);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_normalize_field();

private:
    std::vector<std::pair<std::string, std::string>> test_cases{
        {"Obsoleting packages", "obsoleting_packages"},
        {"ObSoLeTiNg   PaCkAgEs", "obsoleting___packages"},
    };
};


#endif  // TEST_LIBDNF5_CLI_UTILS_JSON_HPP
