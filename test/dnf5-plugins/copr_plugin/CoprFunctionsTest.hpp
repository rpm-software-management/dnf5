// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_TEST_COPR_FUNCTIONS_HPP
#define DNF5_TEST_COPR_FUNCTIONS_HPP

// Note 1
#include <cppunit/extensions/HelperMacros.h>

class CoprFunctionsTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(CoprFunctionsTest);
    CPPUNIT_TEST(test_repo_fallbacks);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {}
    void tearDown() { unsetenv("TEST_COPR_CONFIG_DIR"); }

    void test_repo_fallbacks();
};

#endif  // DNF5_TEST_COPR_FUNCTIONS_HPP
