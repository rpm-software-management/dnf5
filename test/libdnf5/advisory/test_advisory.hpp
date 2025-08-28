// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_ADVISORY_ADVISORY_HPP
#define TEST_LIBDNF5_ADVISORY_ADVISORY_HPP


#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/advisory/advisory_collection.hpp>

class AdvisoryAdvisoryTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(AdvisoryAdvisoryTest);

    CPPUNIT_TEST(test_get_name);
    CPPUNIT_TEST(test_get_type);
    CPPUNIT_TEST(test_get_severity);
    CPPUNIT_TEST(test_get_references);
    CPPUNIT_TEST(test_get_collections);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_get_name();
    void test_get_type();
    void test_get_severity();
    //void test_filter_package();

    void test_get_references();
    void test_get_collections();
};


#endif  // TEST_LIBDNF5_ADVISORY_ADVISORY_HPP
