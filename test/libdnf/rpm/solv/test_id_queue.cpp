/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "test_id_queue.hpp"
#include "../../../../libdnf/rpm/solv/id_queue.hpp"


CPPUNIT_TEST_SUITE_REGISTRATION(IdQueueTest);


void IdQueueTest::setUp() {}


void IdQueueTest::tearDown() {}


// Test push_back(), operator[], size(), and clear()
void IdQueueTest::test_push_back()
{
    libdnf::rpm::solv::IdQueue id_queue;
    CPPUNIT_ASSERT(id_queue.size() == 0);

    id_queue.push_back(4);
    CPPUNIT_ASSERT(id_queue.size() == 1);

    id_queue.push_back(3, 2);
    CPPUNIT_ASSERT(id_queue.size() == 3);

    // insert same valule like it is in queue will result in increase of elements
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
void IdQueueTest::test_operators()
{
    libdnf::rpm::solv::IdQueue id_queue_same1;
    libdnf::rpm::solv::IdQueue id_queue_same2;
    libdnf::rpm::solv::IdQueue id_queue_different3;
    libdnf::rpm::solv::IdQueue id_queue_different4;
    id_queue_same1.push_back(3, 2);
    id_queue_same2.push_back(3, 2);
    id_queue_different3.push_back(2, 5);
    id_queue_different4.push_back(2);

    CPPUNIT_ASSERT(id_queue_same1 == id_queue_same2);
    CPPUNIT_ASSERT(id_queue_same1 == id_queue_same1);
    CPPUNIT_ASSERT(id_queue_same1 != id_queue_different3);
    CPPUNIT_ASSERT(id_queue_different4 != id_queue_different3);
    CPPUNIT_ASSERT(id_queue_different3 != id_queue_different4);

    // test copy costructor
    auto copy = id_queue_same1;
    CPPUNIT_ASSERT(id_queue_same1 == copy);
    CPPUNIT_ASSERT(id_queue_same1.size() == 2);

    // test move costructor
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
