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

#include "test_preserve_order_map.hpp"

#include <libdnf5/common/preserve_order_map.hpp>

#include <string>
#include <vector>


CPPUNIT_TEST_SUITE_REGISTRATION(PreserveOrderMapTest);


void PreserveOrderMapTest::test_basics() {
    // test default constructor
    libdnf5::PreserveOrderMap<int, std::string> map1;
    CPPUNIT_ASSERT(map1.empty());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), map1.size());

    // test insert
    map1.insert({1, "1"});
    CPPUNIT_ASSERT(!map1.empty());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), map1.size());
}

void PreserveOrderMapTest::test_insert() {
    libdnf5::PreserveOrderMap<int, std::string> map1;

    // test insert first item
    // Returns a pair consisting of an iterator to the inserted element and true.
    auto [it1, inserted1] = map1.insert({1, "1"});
    CPPUNIT_ASSERT(inserted1);
    CPPUNIT_ASSERT_EQUAL(1, it1->first);
    CPPUNIT_ASSERT_EQUAL(std::string("1"), it1->second);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), map1.size());

    // test insert second item
    auto [it2, inserted2] = map1.insert({6, "6"});
    CPPUNIT_ASSERT(inserted2);
    CPPUNIT_ASSERT_EQUAL(6, it2->first);
    CPPUNIT_ASSERT_EQUAL(std::string("6"), it2->second);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), map1.size());

    // test insert third item
    auto [it3, inserted3] = map1.insert({4, "4"});
    CPPUNIT_ASSERT(inserted3);
    CPPUNIT_ASSERT_EQUAL(4, it3->first);
    CPPUNIT_ASSERT_EQUAL(std::string("4"), it3->second);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), map1.size());

    // test insert existing
    // Returns a pair consisting of an iterator to the element that prevented the insertion and false.
    auto [it4, inserted4] = map1.insert({6, "6M"});
    CPPUNIT_ASSERT(!inserted4);
    CPPUNIT_ASSERT_EQUAL(6, it4->first);
    CPPUNIT_ASSERT_EQUAL(std::string("6"), it4->second);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), map1.size());
}

void PreserveOrderMapTest::test_count() {
    libdnf5::PreserveOrderMap<int, std::string> map1;
    map1.insert({1, "1"});
    map1.insert({5, "5"});

    // test count existing items
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), map1.count(5));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), map1.count(1));

    // test count non existing item
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), map1.count(20));
}

void PreserveOrderMapTest::test_copy_move() {
    libdnf5::PreserveOrderMap<int, std::string> map1;
    map1.insert({1, "1"});
    map1.insert({5, "5"});

    // test copy constructor
    libdnf5::PreserveOrderMap<int, std::string> map2 = map1;
    CPPUNIT_ASSERT(!map2.empty());
    CPPUNIT_ASSERT_EQUAL(map2.size(), static_cast<size_t>(2));
    CPPUNIT_ASSERT(map2.count(1) == 1 && map2.count(5) == 1 && map2.count(8) == 0);

    // test move constructor
    libdnf5::PreserveOrderMap<int, std::string> map3 = std::move(map2);
    CPPUNIT_ASSERT(!map3.empty());
    CPPUNIT_ASSERT(map2.empty());
    CPPUNIT_ASSERT_EQUAL(map3.size(), static_cast<size_t>(2));
    CPPUNIT_ASSERT(map3.count(1) == 1 && map3.count(5) == 1 && map3.count(8) == 0);
}

void PreserveOrderMapTest::test_find() {
    libdnf5::PreserveOrderMap<int, std::string> map1;
    map1.insert({1, "1"});
    map1.insert({8, "8"});
    map1.insert({5, "5"});

    // test find existing item
    auto it = map1.find(8);
    CPPUNIT_ASSERT_EQUAL(8, it->first);
    CPPUNIT_ASSERT_EQUAL(std::string("8"), it->second);

    // test find non existing item
    CPPUNIT_ASSERT(map1.find(4) == map1.end());
}

void PreserveOrderMapTest::test_access() {
    libdnf5::PreserveOrderMap<int, std::string> map1;
    map1.insert({1, "1"});
    map1.insert({5, "5"});

    // operator[] read existing item
    CPPUNIT_ASSERT_EQUAL(std::string("5"), map1[5]);

    // operator[] add item and read them back
    map1[4] = "4";
    CPPUNIT_ASSERT_EQUAL(std::string("4"), map1[4]);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), map1.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), map1.count(4));

    // operator[] modify existing item
    map1[4] = "4M";
    CPPUNIT_ASSERT_EQUAL(std::string("4M"), map1[4]);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), map1.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), map1.count(4));

    // test at, read existing item
    CPPUNIT_ASSERT_EQUAL(std::string("5"), map1.at(5));

    // test at, modify existing item
    map1.at(5) = "5M";
    CPPUNIT_ASSERT_EQUAL(std::string("5M"), map1.at(5));

    // test at, non existing item throw exception
    CPPUNIT_ASSERT_THROW(map1.at(2), std::out_of_range);
}

void PreserveOrderMapTest::test_erase_clear() {
    libdnf5::PreserveOrderMap<int, int> map1;
    for (int i = 0; i < 20; ++i) {
        map1.insert({i, i});
    }

    // test erase existing item with given key
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), map1.erase(2));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(19), map1.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), map1.count(2));

    // test erase non existing item with given key
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), map1.erase(25));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(19), map1.size());

    // test erase existing item using iterator
    auto it = map1.erase(map1.find(5));
    CPPUNIT_ASSERT(it == map1.find(6));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(18), map1.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), map1.count(5));

    // test erase range of elements
    auto it2 = map1.erase(map1.find(10), map1.find(16));
    CPPUNIT_ASSERT(it2 == map1.find(16));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(12), map1.size());

    // test clear()
    map1.clear();
    CPPUNIT_ASSERT(map1.empty());
}

void PreserveOrderMapTest::test_iterators() {
    libdnf5::PreserveOrderMap<int, std::string> map1;
    map1.insert({1, "1"});
    map1.insert({6, "6"});
    map1[4] = "4";
    map1[5] = "5";

    // check if begin() points to the first package
    auto it1 = map1.begin();
    CPPUNIT_ASSERT_EQUAL(1, it1->first);

    // test pre-increment operator
    auto it2 = ++it1;
    CPPUNIT_ASSERT_EQUAL(6, it1->first);
    CPPUNIT_ASSERT_EQUAL(6, it2->first);

    // test post-increment operator
    auto it3 = it2++;
    CPPUNIT_ASSERT_EQUAL(4, it2->first);
    CPPUNIT_ASSERT_EQUAL(6, it3->first);

    // test pre-decrement operator
    auto it4 = --it2;
    CPPUNIT_ASSERT_EQUAL(6, it2->first);
    CPPUNIT_ASSERT_EQUAL(6, it4->first);

    // test post-decrement operator
    auto it5 = it4--;
    CPPUNIT_ASSERT_EQUAL(1, it4->first);
    CPPUNIT_ASSERT_EQUAL(6, it5->first);

    // test copy assignment
    it4 = it5;
    CPPUNIT_ASSERT_EQUAL(6, it4->first);

    // test iterator to const_iterator conversion
    auto const_it = map1.cbegin();
    CPPUNIT_ASSERT_EQUAL(1, const_it->first);
    const_it = it4;
    CPPUNIT_ASSERT_EQUAL(6, const_it->first);

    // test loop
    {
        std::vector<int> expected{1, 6, 4, 5};
        std::vector<int> result;
        for (auto & value : map1) {
            result.push_back(value.first);
        }
        CPPUNIT_ASSERT(result == expected);
    }

    // test const loop
    {
        std::vector<int> expected{1, 6, 4, 5};
        std::vector<int> result;
        for (auto it = map1.cbegin(); it != map1.cend(); ++it) {
            result.push_back(it->first);
        }
        CPPUNIT_ASSERT(result == expected);
    }

    // test reverse loop
    {
        std::vector<int> expected{5, 4, 6, 1};
        std::vector<int> result;
        for (auto it = map1.rbegin(); it != map1.rend(); ++it) {
            result.push_back(it->first);
        }
        CPPUNIT_ASSERT(result == expected);
    }

    // test const reverse loop
    {
        std::vector<int> expected{5, 4, 6, 1};
        std::vector<int> result;
        for (auto it = map1.crbegin(); it != map1.crend(); ++it) {
            result.push_back(it->first);
        }
        CPPUNIT_ASSERT(result == expected);
    }
}
