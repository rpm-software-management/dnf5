// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_ARCH_HPP
#define LIBDNF5_TEST_ARCH_HPP

#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>


class ArchTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(ArchTest);
    CPPUNIT_TEST(test_get_base_arch);
    CPPUNIT_TEST(test_get_supported_arches);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_get_base_arch();
    void test_get_supported_arches();
};

#endif  // TEST_LIBDNF5_RPM_ARCH_HPP
