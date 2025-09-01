// Copyright Contributors to the DNF5 project.
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


#include "test_id_queue.hpp"

#include "solv/id_queue.hpp"

#include <iterator>

CPPUNIT_TEST_SUITE_REGISTRATION(IdQueueTest);


void IdQueueTest::setUp() {}


void IdQueueTest::tearDown() {}


// Test push_back(), operator[], size(), and clear()
void IdQueueTest::test_push_back() {
    libdnf5::solv::IdQueue id_queue;
    CPPUNIT_ASSERT(id_queue.size() == 0);

    id_queue.push_back(4);
    CPPUNIT_ASSERT(id_queue.size() == 1);

    id_queue.push_back(3, 2);
    CPPUNIT_ASSERT(id_queue.size() == 3);

    // insert same value like it is in queue will result in increase of elements
    id_queue.push_back(3);
    CPPUNIT_ASSERT(id_queue.size() == 4);

    // check the order of inserted values
    CPPUNIT_ASSERT(id_queue[0] == 4);
    CPPUNIT_ASSERT(id_queue[1] == 3);
    CPPUNIT_ASSERT(id_queue[2] == 2);
    CPPUNIT_ASSERT(id_queue[3] == 3);
    id_queue.clear();
    CPPUNIT_ASSERT(id_queue.size() == 0);
}

/// Test operators, copy and move constructors
void IdQueueTest::test_operators() {
    libdnf5::solv::IdQueue id_queue_same1;
    libdnf5::solv::IdQueue id_queue_same2;
    libdnf5::solv::IdQueue id_queue_different3;
    libdnf5::solv::IdQueue id_queue_different4;
    id_queue_same1.push_back(3, 2);
    id_queue_same2.push_back(3, 2);
    id_queue_different3.push_back(2, 5);
    id_queue_different4.push_back(2);

    CPPUNIT_ASSERT(id_queue_same1 == id_queue_same2);
    CPPUNIT_ASSERT(id_queue_same1 == id_queue_same1);
    CPPUNIT_ASSERT(id_queue_same1 != id_queue_different3);
    CPPUNIT_ASSERT(id_queue_different4 != id_queue_different3);
    CPPUNIT_ASSERT(id_queue_different3 != id_queue_different4);

    // test copy constructor
    auto copy = id_queue_same1;
    CPPUNIT_ASSERT(id_queue_same1 == copy);
    CPPUNIT_ASSERT(id_queue_same1.size() == 2);

    // test move constructor
    auto move = std::move(id_queue_same1);
    CPPUNIT_ASSERT(id_queue_same1 != move);
    CPPUNIT_ASSERT(id_queue_same1.size() == 0);
    CPPUNIT_ASSERT(move.size() == 2);

    /// test += operator
    CPPUNIT_ASSERT(id_queue_same2.size() == 2);
    CPPUNIT_ASSERT(id_queue_different3.size() == 2);
    id_queue_same2 += id_queue_different3;
    CPPUNIT_ASSERT(id_queue_same2.size() == 4);
    CPPUNIT_ASSERT(id_queue_different3.size() == 2);
    CPPUNIT_ASSERT(id_queue_same2[0] == 3);
    CPPUNIT_ASSERT(id_queue_same2[1] == 2);
    CPPUNIT_ASSERT(id_queue_same2[2] == 2);
    CPPUNIT_ASSERT(id_queue_same2[3] == 5);
}

void IdQueueTest::test_iterator_empty() {
    std::vector<Id> expected = {};
    std::vector<Id> result;
    libdnf5::solv::IdQueue queue;

    for (auto it = queue.begin(); it != queue.end(); it++) {
        result.push_back(*it);
    }
    CPPUNIT_ASSERT(result == expected);
}

void IdQueueTest::test_iterator_full() {
    std::vector<Id> expected = {5, 6, 8, 10, 14};
    libdnf5::solv::IdQueue queue;
    queue.push_back(5, 6);
    queue.push_back(8, 10);
    queue.push_back(14);

    // test begin
    auto it1 = queue.begin();
    CPPUNIT_ASSERT_EQUAL(*it1, 5);

    // test pre-increment operator
    auto it2 = ++it1;
    CPPUNIT_ASSERT_EQUAL(*it1, 6);
    CPPUNIT_ASSERT_EQUAL(*it2, 6);

    // test post-increment operator
    auto it3 = it2++;
    CPPUNIT_ASSERT_EQUAL(*it2, 8);
    CPPUNIT_ASSERT_EQUAL(*it3, 6);

    // test move back to the begin
    it2.begin();
    CPPUNIT_ASSERT_EQUAL(*it2, 5);
    CPPUNIT_ASSERT(it2 == queue.begin());

    // test increment after begin
    ++it2;
    CPPUNIT_ASSERT_EQUAL(*it2, 6);

    auto it4 = std::next(it2);
    CPPUNIT_ASSERT_EQUAL(*it2, 6);
    CPPUNIT_ASSERT_EQUAL(*it4, 8);

    std::advance(it4, 2);
    CPPUNIT_ASSERT_EQUAL(*it4, 14);

    // test move to the end
    it2.end();
    CPPUNIT_ASSERT(it2 == queue.end());

    // test loop with pre-increment operator
    {
        std::vector<Id> result;
        for (auto it = queue.begin(), end = queue.end(); it != end; ++it) {
            result.push_back(*it);
        }
        CPPUNIT_ASSERT(result == expected);
    }

    // test loop with post-increment operator
    {
        std::vector<Id> result;
        for (auto it = queue.begin(), end = queue.end(); it != end; it++) {
            result.push_back(*it);
        }
        CPPUNIT_ASSERT(result == expected);
    }
}

void IdQueueTest::test_iterator_performance() {
    constexpr int max = 1000000;
    libdnf5::solv::IdQueue queue;
    for (int i = 0; i < max; i++) {
        queue.push_back(i);
    }

    for (int i = 0; i < 10; i++) {
        std::vector<Id> result;
        for (auto it = queue.begin(); it != queue.end(); it++) {
            result.push_back(*it);
        }
    }
}
