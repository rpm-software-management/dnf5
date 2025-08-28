// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_ADVISORY_ADVISORY_SET_HPP
#define TEST_LIBDNF5_ADVISORY_ADVISORY_SET_HPP


#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/advisory/advisory_set.hpp>

#include <memory>


class AdvisoryAdvisorySetTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(AdvisoryAdvisorySetTest);

#ifndef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_add);
    CPPUNIT_TEST(test_contains);
    CPPUNIT_TEST(test_remove);
    CPPUNIT_TEST(test_union);
    CPPUNIT_TEST(test_intersection);
    CPPUNIT_TEST(test_difference);
    CPPUNIT_TEST(test_iterator);
#endif

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_add();
    void test_contains();
    void test_remove();

    void test_union();
    void test_intersection();
    void test_difference();

    void test_iterator();


private:
    std::unique_ptr<libdnf5::advisory::AdvisorySet> set1;
    std::unique_ptr<libdnf5::advisory::AdvisorySet> set2;
};


#endif  // TEST_LIBDNF5_ADVISORY_ADVISORY_SET_HPP
