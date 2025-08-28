// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_SOLV_MAP_HPP
#define TEST_LIBDNF5_SOLV_MAP_HPP


#include "solv/solv_map.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class SolvMapTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(SolvMapTest);

#ifndef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_add);
    CPPUNIT_TEST(test_contains);
    CPPUNIT_TEST(test_remove);
    CPPUNIT_TEST(test_map_allocation_range);
    CPPUNIT_TEST(test_union);
    CPPUNIT_TEST(test_intersection);
    CPPUNIT_TEST(test_difference);
    CPPUNIT_TEST(test_iterator_empty);
    CPPUNIT_TEST(test_iterator_full);
    CPPUNIT_TEST(test_iterator_sparse);
#endif

#ifdef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_iterator_performance_empty);
    CPPUNIT_TEST(test_iterator_performance_full);
    CPPUNIT_TEST(test_iterator_performance_4bits);
#endif

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_add();
    void test_contains();
    void test_remove();

    void test_map_allocation_range();

    void test_union();
    void test_intersection();
    void test_difference();

    void test_iterator_empty();
    void test_iterator_full();
    void test_iterator_sparse();

    void test_iterator_performance_empty();
    void test_iterator_performance_full();
    void test_iterator_performance_4bits();

private:
    libdnf5::solv::SolvMap * map1;
    libdnf5::solv::SolvMap * map2;
};


#endif  // TEST_LIBDNF5_SOLV_MAP_HPP
