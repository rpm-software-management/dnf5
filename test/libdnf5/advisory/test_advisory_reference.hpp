// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_ADVISORY_ADVISORY_REFERENCE_HPP
#define TEST_LIBDNF5_ADVISORY_ADVISORY_REFERENCE_HPP


#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/advisory/advisory_collection.hpp>
#include <libdnf5/advisory/advisory_reference.hpp>

class AdvisoryAdvisoryReferenceTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(AdvisoryAdvisoryReferenceTest);

    CPPUNIT_TEST(test_get_id);
    CPPUNIT_TEST(test_get_type);
    CPPUNIT_TEST(test_get_title);
    CPPUNIT_TEST(test_get_url);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_get_id();
    void test_get_type();
    void test_get_title();
    void test_get_url();

private:
    std::vector<libdnf5::advisory::AdvisoryReference> references;
};


#endif  // TEST_LIBDNF5_ADVISORY_ADVISORY_REFERENCE_HPP
