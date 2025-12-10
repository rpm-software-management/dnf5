// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "test_json.hpp"

#include "libdnf5-cli/utils/json.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(JSONTest);

void JSONTest::setUp() {}
void JSONTest::tearDown() {}

void JSONTest::test_normalize_field() {
    for (auto && [field_name, expected] : test_cases) {
        CPPUNIT_ASSERT_EQUAL(expected, libdnf5::cli::utils::json::normalize_field(field_name));
    }
}
