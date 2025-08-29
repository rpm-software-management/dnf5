// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "test_patterns.hpp"

#include <libdnf5/utils/patterns.hpp>

using namespace libdnf5::utils;

CPPUNIT_TEST_SUITE_REGISTRATION(UtilsPatternsTest);


void UtilsPatternsTest::test_is_file_pattern() {
    CPPUNIT_ASSERT(!is_file_pattern(""));
    CPPUNIT_ASSERT(!is_file_pattern("no_file_pattern"));
    CPPUNIT_ASSERT(!is_file_pattern("no_file/pattern"));
    CPPUNIT_ASSERT(is_file_pattern("/pattern"));
    CPPUNIT_ASSERT(is_file_pattern("*/pattern"));
}


void UtilsPatternsTest::test_is_glob_pattern() {
    CPPUNIT_ASSERT(!is_glob_pattern(""));
    CPPUNIT_ASSERT(!is_glob_pattern("no_glob_pattern"));
    CPPUNIT_ASSERT(is_glob_pattern("glob*_pattern"));
    CPPUNIT_ASSERT(is_glob_pattern("glob[sdf]_pattern"));
    CPPUNIT_ASSERT(is_glob_pattern("glob?_pattern"));
}
