// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_PRESERVE_ORDER_MAP_HPP
#define LIBDNF5_TEST_PRESERVE_ORDER_MAP_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class PreserveOrderMapTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(PreserveOrderMapTest);
    CPPUNIT_TEST(test_basics);
    CPPUNIT_TEST(test_insert);
    CPPUNIT_TEST(test_count);
    CPPUNIT_TEST(test_copy_move);
    CPPUNIT_TEST(test_find);
    CPPUNIT_TEST(test_access);
    CPPUNIT_TEST(test_erase_clear);
    CPPUNIT_TEST(test_iterators);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_basics();
    void test_insert();
    void test_count();
    void test_copy_move();
    void test_find();
    void test_access();
    void test_erase_clear();
    void test_iterators();
};

#endif
