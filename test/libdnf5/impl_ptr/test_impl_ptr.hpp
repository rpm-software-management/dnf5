// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_IMPL_PTR_HPP
#define LIBDNF5_TEST_IMPL_PTR_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class ImplPtrTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(ImplPtrTest);
    CPPUNIT_TEST(test_default_constructor);
    CPPUNIT_TEST(test_constructor_from_pointer);
    CPPUNIT_TEST(test_access_to_managed_object);
    CPPUNIT_TEST(test_const_access_to_managed_object);
    CPPUNIT_TEST(test_copy_constructor);
    CPPUNIT_TEST(test_move_constructor);
    CPPUNIT_TEST(test_copy_assignment);
    CPPUNIT_TEST(test_move_assignment);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_default_constructor();
    void test_constructor_from_pointer();
    void test_access_to_managed_object();
    void test_const_access_to_managed_object();
    void test_copy_constructor();
    void test_move_constructor();
    void test_copy_assignment();
    void test_move_assignment();
};

#endif
