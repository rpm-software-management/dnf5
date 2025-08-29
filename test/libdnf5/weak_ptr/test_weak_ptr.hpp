// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_WEAK_PTR_HPP
#define LIBDNF5_TEST_WEAK_PTR_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class WeakPtrTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(WeakPtrTest);
    CPPUNIT_TEST(test_weak_ptr);
    CPPUNIT_TEST(test_weak_ptr_is_owner);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_weak_ptr();
    void test_weak_ptr_is_owner();

private:
};

#endif
