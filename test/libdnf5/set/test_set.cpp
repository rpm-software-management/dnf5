// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "test_set.hpp"

#include <libdnf5/common/set.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION(SetTest);


void SetTest::setUp() {}


void SetTest::tearDown() {}

// Test constructors, empty(), size(), contains(), clear(), add(), remove(), and find()
void SetTest::test_set_basics() {
    // test default constructor
    libdnf5::Set<int> s1;
    CPPUNIT_ASSERT(s1.empty());
    CPPUNIT_ASSERT_EQUAL(s1.size(), static_cast<size_t>(0));

    // test initializer list constructor
    libdnf5::Set<int> s2{1, 5};
    CPPUNIT_ASSERT(!s2.empty());
    CPPUNIT_ASSERT_EQUAL(s2.size(), static_cast<size_t>(2));
    CPPUNIT_ASSERT(s2.contains(1) && s2.contains(5) && !s2.contains(8));

    // test copy constructor
    libdnf5::Set<int> s3 = s2;
    CPPUNIT_ASSERT(!s3.empty());
    CPPUNIT_ASSERT_EQUAL(s3.size(), static_cast<size_t>(2));
    CPPUNIT_ASSERT(s3.contains(1) && s3.contains(5) && !s3.contains(8));

    // test move constructor
    libdnf5::Set<int> s4 = std::move(s2);
    CPPUNIT_ASSERT(!s4.empty());
    CPPUNIT_ASSERT_EQUAL(s4.size(), static_cast<size_t>(2));
    CPPUNIT_ASSERT(s4.contains(1) && s4.contains(5) && !s4.contains(8));

    // test clear()
    s4.clear();
    CPPUNIT_ASSERT(s1.empty());

    s1 = {1, 4, 6};
    CPPUNIT_ASSERT(s1.add(2));
    CPPUNIT_ASSERT((s1 == libdnf5::Set<int>{1, 2, 4, 6}));

    s1 = {1, 4, 6};
    CPPUNIT_ASSERT(!s1.add(4));
    CPPUNIT_ASSERT((s1 == libdnf5::Set<int>{1, 4, 6}));

    s1 = {1, 4, 6};
    CPPUNIT_ASSERT(s1.remove(4));
    CPPUNIT_ASSERT((s1 == libdnf5::Set<int>{1, 6}));

    s1 = {1, 4, 6};
    CPPUNIT_ASSERT(!s1.remove(5));
    CPPUNIT_ASSERT((s1 == libdnf5::Set<int>{1, 4, 6}));

    s1 = {1, 4, 6};
    libdnf5::Set<int>::iterator it = s1.find(4);
    CPPUNIT_ASSERT_EQUAL(*it, 4);
    CPPUNIT_ASSERT(s1.find(5) == s1.end());
}

// test operator ==()
void SetTest::test_set_equal_operator() {
    CPPUNIT_ASSERT((libdnf5::Set<int>{1, 2, 4} == libdnf5::Set<int>{1, 2, 4}));
    CPPUNIT_ASSERT((libdnf5::Set<int>{1, 2, 4} == libdnf5::Set<int>{1, 4, 2}));
    CPPUNIT_ASSERT((libdnf5::Set<int>{1, 2, 4} == libdnf5::Set<int>{1, 4, 2, 1}));
    CPPUNIT_ASSERT(!(libdnf5::Set<int>{1, 2, 4} == libdnf5::Set<int>{1, 4, 4}));
}

// test operator =()
void SetTest::test_set_assignment_operator() {
    libdnf5::Set<int> s1;
    libdnf5::Set<int> s2;

    s1 = {1, 4};
    CPPUNIT_ASSERT((s1 == libdnf5::Set<int>{1, 4}));

    s2 = s1;
    CPPUNIT_ASSERT((s2 == libdnf5::Set<int>{1, 4}));

    s2 = std::move(s1);
    CPPUNIT_ASSERT((s2 == libdnf5::Set<int>{1, 4}));
}

// test unary operators |= &= -= ^=
void SetTest::test_set_unary_operators() {
    libdnf5::Set<int> s;

    s = {1, 4};
    s |= {2, 4, 5};
    CPPUNIT_ASSERT((s == libdnf5::Set<int>{1, 2, 4, 5}));

    s = {1, 2, 3, 4};
    s &= {2, 4, 6};
    CPPUNIT_ASSERT((s == libdnf5::Set<int>{2, 4}));

    s = {1, 4, 6};
    s -= {2, 4, 5};
    CPPUNIT_ASSERT((s == libdnf5::Set<int>{1, 6}));

    s = {1, 2, 3, 4};
    s ^= {2, 4, 6};
    CPPUNIT_ASSERT((s == libdnf5::Set<int>{1, 3, 6}));
}

// test unary methods update(), intersection(), difference(), symetric_difference(), and swap()
void SetTest::test_set_unary_methods() {
    libdnf5::Set<int> s;

    s = {1, 4};
    s.update({2, 4, 5});
    CPPUNIT_ASSERT((s == libdnf5::Set<int>{1, 2, 4, 5}));

    s = {1, 2, 3, 4};
    s.intersection({2, 4, 6});
    CPPUNIT_ASSERT((s == libdnf5::Set<int>{2, 4}));

    s = {1, 4, 6};
    s.difference({2, 4, 5});
    CPPUNIT_ASSERT((s == libdnf5::Set<int>{1, 6}));

    s = {1, 2, 3, 4};
    s.symmetric_difference({2, 4, 6});
    CPPUNIT_ASSERT((s == libdnf5::Set<int>{1, 3, 6}));

    libdnf5::Set<int> s1{1, 4, 6};
    libdnf5::Set<int> s2{2, 5, 8};
    s1.swap(s2);
    CPPUNIT_ASSERT((s1 == libdnf5::Set<int>{2, 5, 8}));
    CPPUNIT_ASSERT((s2 == libdnf5::Set<int>{1, 4, 6}));
}

// test binary operators | & - ^
void SetTest::test_set_binary_operators() {
    // test binary operators | & - ^
    libdnf5::Set<int> s1{1, 4, 6};
    libdnf5::Set<int> s2{2, 4};
    CPPUNIT_ASSERT(((s1 | s2) == libdnf5::Set<int>{1, 2, 4, 6}));
    CPPUNIT_ASSERT(((s1 & s2) == libdnf5::Set<int>{4}));
    CPPUNIT_ASSERT(((s1 - s2) == libdnf5::Set<int>{1, 6}));
    CPPUNIT_ASSERT(((s1 ^ s2) == libdnf5::Set<int>{1, 2, 6}));
}

// test iterator
void SetTest::test_set_iterator() {
    libdnf5::Set<int> s1{1, 4, 5, 6};

    // check if begin() points to the first package
    auto it1 = s1.begin();
    CPPUNIT_ASSERT_EQUAL(*it1, 1);

    // test pre-increment operator
    auto it2 = ++it1;
    CPPUNIT_ASSERT_EQUAL(*it1, 4);
    CPPUNIT_ASSERT_EQUAL(*it2, 4);

    // test post-increment operator
    auto it3 = it2++;
    CPPUNIT_ASSERT_EQUAL(*it2, 5);
    CPPUNIT_ASSERT_EQUAL(*it3, 4);

    // test pre-decrement operator
    auto it4 = --it2;
    CPPUNIT_ASSERT_EQUAL(*it2, 4);
    CPPUNIT_ASSERT_EQUAL(*it4, 4);

    // test post-decrement operator
    auto it5 = it4--;
    CPPUNIT_ASSERT_EQUAL(*it4, 1);
    CPPUNIT_ASSERT_EQUAL(*it5, 4);

    // test loop
    {
        std::vector<int> expected{1, 4, 5, 6};
        std::vector<int> result;
        for (auto value : s1) {
            result.push_back(value);
        }
        CPPUNIT_ASSERT(result == expected);
    }
}
