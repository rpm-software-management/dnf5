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

#include "test_weak_ptr.hpp"

#include <libdnf5/common/weak_ptr.hpp>

#include <memory>
#include <string>
#include <vector>

CPPUNIT_TEST_SUITE_REGISTRATION(WeakPtrTest);


void WeakPtrTest::setUp() {}


void WeakPtrTest::tearDown() {}


// In this test the WeakPtr instances point to data owned by Sack instance.
void WeakPtrTest::test_weak_ptr() {
    class Sack {
    public:
        using DataItemWeakPtr = libdnf5::WeakPtr<std::string, false>;

        DataItemWeakPtr add_item_with_return(std::unique_ptr<std::string> && item) {
            auto ret = DataItemWeakPtr(item.get(), &data_guard);
            data.push_back(std::move(item));
            return ret;
        }

        libdnf5::WeakPtrGuard<std::string, false> data_guard;

    private:
        std::vector<std::unique_ptr<std::string>>
            data;  // Owns the data set. Objects get deleted when the Sack is deleted.
    };

    // add weak pointers, test WeakPtrGuard::empty(), and WeakPtrGuard::size()
    auto sack1 = std::make_unique<Sack>();
    CPPUNIT_ASSERT(sack1->data_guard.empty());
    CPPUNIT_ASSERT_EQUAL(sack1->data_guard.size(), static_cast<std::size_t>(0));
    auto item1_weak_ptr = sack1->add_item_with_return(std::make_unique<std::string>("sack1_item1"));
    auto item2_weak_ptr = sack1->add_item_with_return(std::make_unique<std::string>("sack1_item2"));
    CPPUNIT_ASSERT(!sack1->data_guard.empty());
    CPPUNIT_ASSERT_EQUAL(sack1->data_guard.size(), static_cast<std::size_t>(2));

    auto sack2 = std::make_unique<Sack>();
    auto item3_weak_ptr = sack2->add_item_with_return(std::make_unique<std::string>("sack2_item1"));
    auto item4_weak_ptr = sack2->add_item_with_return(std::make_unique<std::string>("sack2_item2"));

    // test access to data managed by WeakPtr (by get() and operator ->)
    CPPUNIT_ASSERT(*item1_weak_ptr.get() == "sack1_item1");
    CPPUNIT_ASSERT(item2_weak_ptr->compare("sack1_item2") == 0);
    CPPUNIT_ASSERT(*item3_weak_ptr.get() == "sack2_item1");
    CPPUNIT_ASSERT(item4_weak_ptr->compare("sack2_item2") == 0);

    // test hase_same_guard() method
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr.has_same_guard(item1_weak_ptr), true);
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr.has_same_guard(item2_weak_ptr), true);
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr.has_same_guard(item3_weak_ptr), false);

    // delete sack2
    sack2.reset();

    // data from sack1 must be still accessible, but access to data from sack2 must throw exception
    CPPUNIT_ASSERT(*item1_weak_ptr.get() == "sack1_item1");
    CPPUNIT_ASSERT(item2_weak_ptr->compare("sack1_item2") == 0);
    CPPUNIT_ASSERT_THROW(static_cast<void>(*item3_weak_ptr.get() == "sack2_item1"), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(static_cast<void>(item4_weak_ptr->compare("sack2_item2") == 0), libdnf5::AssertionError);

    // test is_valid() method
    CPPUNIT_ASSERT(item1_weak_ptr.is_valid());
    CPPUNIT_ASSERT(item2_weak_ptr.is_valid());
    CPPUNIT_ASSERT(!item3_weak_ptr.is_valid());
    CPPUNIT_ASSERT(!item4_weak_ptr.is_valid());

    // test copy constructor
    auto item5_weak_ptr(item1_weak_ptr);
    CPPUNIT_ASSERT_EQUAL(sack1->data_guard.size(), static_cast<std::size_t>(3));
    CPPUNIT_ASSERT(*item1_weak_ptr.get() == "sack1_item1");
    CPPUNIT_ASSERT(*item5_weak_ptr.get() == "sack1_item1");

    // there is no move constructor, copy constructor must be used
    auto item6_weak_ptr(std::move(item5_weak_ptr));
    CPPUNIT_ASSERT_EQUAL(sack1->data_guard.size(), static_cast<std::size_t>(4));
    CPPUNIT_ASSERT(*item5_weak_ptr.get() == "sack1_item1");
    CPPUNIT_ASSERT(*item6_weak_ptr.get() == "sack1_item1");

    // test copy assignment operator =
    item3_weak_ptr = item1_weak_ptr;
    CPPUNIT_ASSERT_EQUAL(sack1->data_guard.size(), static_cast<std::size_t>(5));
    CPPUNIT_ASSERT(*item1_weak_ptr.get() == "sack1_item1");
    CPPUNIT_ASSERT(*item3_weak_ptr.get() == "sack1_item1");

    // there is no move assignment operator =, copy assignment must be used
    item4_weak_ptr = std::move(item2_weak_ptr);
    CPPUNIT_ASSERT_EQUAL(sack1->data_guard.size(), static_cast<std::size_t>(6));
    CPPUNIT_ASSERT(*item2_weak_ptr.get() == "sack1_item2");
    CPPUNIT_ASSERT(*item4_weak_ptr.get() == "sack1_item2");

    // test operator ==
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr == item1_weak_ptr, true);
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr == item3_weak_ptr, true);
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr == item4_weak_ptr, false);

    // test operator !=
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr != item1_weak_ptr, false);
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr != item3_weak_ptr, false);
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr != item4_weak_ptr, true);

    // test WeakPtrGuard::clear()
    // It must deregister and invalidate all registered weak pointers.
    CPPUNIT_ASSERT_EQUAL(sack1->data_guard.size(), static_cast<std::size_t>(6));
    CPPUNIT_ASSERT(*item1_weak_ptr.get() == "sack1_item1");
    CPPUNIT_ASSERT(*item2_weak_ptr.get() == "sack1_item2");
    CPPUNIT_ASSERT(*item3_weak_ptr.get() == "sack1_item1");
    CPPUNIT_ASSERT(*item4_weak_ptr.get() == "sack1_item2");
    CPPUNIT_ASSERT(*item5_weak_ptr.get() == "sack1_item1");
    CPPUNIT_ASSERT(*item6_weak_ptr.get() == "sack1_item1");
    sack1->data_guard.clear();
    CPPUNIT_ASSERT(sack1->data_guard.empty());
    CPPUNIT_ASSERT_EQUAL(sack1->data_guard.size(), static_cast<std::size_t>(0));
    CPPUNIT_ASSERT_THROW(static_cast<void>(*item1_weak_ptr.get() == "sack1_item1"), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(static_cast<void>(*item2_weak_ptr.get() == "sack1_item2"), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(static_cast<void>(*item3_weak_ptr.get() == "sack1_item2"), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(static_cast<void>(*item4_weak_ptr.get() == "sack1_item1"), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(static_cast<void>(*item5_weak_ptr.get() == "sack1_item1"), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(static_cast<void>(*item6_weak_ptr.get() == "sack1_item1"), libdnf5::AssertionError);
}


// In this test the WeakPtr instances own DependentItem instances that point to data owned by Sack instance.
void WeakPtrTest::test_weak_ptr_is_owner() {
    struct DependentItem {
        std::string * remote_data;
    };

    class Sack {
    public:
        using DataItemWeakPtr = libdnf5::WeakPtr<DependentItem, true>;

        DataItemWeakPtr add_item_with_return(std::unique_ptr<std::string> && item) {
            auto ret = DataItemWeakPtr(new DependentItem{item.get()}, &data_guard);
            data.push_back(std::move(item));
            return ret;
        }

        libdnf5::WeakPtrGuard<DependentItem, true> data_guard;

    private:
        std::vector<std::unique_ptr<std::string>>
            data;  // Owns the data set. Objects get deleted when the Sack is deleted.
    };

    // add weak pointers, test WeakPtrGuard::empty(), and WeakPtrGuard::size()
    auto sack1 = std::make_unique<Sack>();
    CPPUNIT_ASSERT(sack1->data_guard.empty());
    CPPUNIT_ASSERT_EQUAL(sack1->data_guard.size(), static_cast<std::size_t>(0));
    auto item1_weak_ptr = sack1->add_item_with_return(std::make_unique<std::string>("sack1_item1"));
    auto item2_weak_ptr = sack1->add_item_with_return(std::make_unique<std::string>("sack1_item2"));
    CPPUNIT_ASSERT(!sack1->data_guard.empty());
    CPPUNIT_ASSERT_EQUAL(sack1->data_guard.size(), static_cast<std::size_t>(2));

    auto sack2 = std::make_unique<Sack>();
    auto item3_weak_ptr = sack2->add_item_with_return(std::make_unique<std::string>("sack2_item1"));
    auto item4_weak_ptr = sack2->add_item_with_return(std::make_unique<std::string>("sack2_item2"));

    // test access to data managed by WeakPtr (by get() and operator ->)
    CPPUNIT_ASSERT(*item1_weak_ptr.get()->remote_data == "sack1_item1");
    CPPUNIT_ASSERT(*item2_weak_ptr->remote_data == "sack1_item2");
    CPPUNIT_ASSERT(*item3_weak_ptr.get()->remote_data == "sack2_item1");
    CPPUNIT_ASSERT(*item4_weak_ptr->remote_data == "sack2_item2");

    // test hase_same_guard() method
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr.has_same_guard(item1_weak_ptr), true);
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr.has_same_guard(item2_weak_ptr), true);
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr.has_same_guard(item3_weak_ptr), false);

    // delete sack2
    sack2.reset();

    // data from sack1 must be still accessible, but access to data from sack2 must throw exception
    CPPUNIT_ASSERT(*item1_weak_ptr.get()->remote_data == "sack1_item1");
    CPPUNIT_ASSERT(*item2_weak_ptr->remote_data == "sack1_item2");
    CPPUNIT_ASSERT_THROW(
        static_cast<void>(*item3_weak_ptr.get()->remote_data == "sack2_item1"), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(static_cast<void>(*item4_weak_ptr->remote_data == "sack2_item2"), libdnf5::AssertionError);

    // test is_valid() method
    CPPUNIT_ASSERT(item1_weak_ptr.is_valid());
    CPPUNIT_ASSERT(item2_weak_ptr.is_valid());
    CPPUNIT_ASSERT(!item3_weak_ptr.is_valid());
    CPPUNIT_ASSERT(!item4_weak_ptr.is_valid());

    // test copy constructor
    auto item5_weak_ptr(item1_weak_ptr);
    CPPUNIT_ASSERT_EQUAL(sack1->data_guard.size(), static_cast<std::size_t>(3));
    CPPUNIT_ASSERT(*item1_weak_ptr->remote_data == "sack1_item1");
    CPPUNIT_ASSERT(*item5_weak_ptr->remote_data == "sack1_item1");

    // there move constructor
    auto item6_weak_ptr(std::move(item5_weak_ptr));
    CPPUNIT_ASSERT_EQUAL(sack1->data_guard.size(), static_cast<std::size_t>(3));
    CPPUNIT_ASSERT_THROW(static_cast<void>(*item5_weak_ptr->remote_data == "sack1_item1"), libdnf5::AssertionError);
    CPPUNIT_ASSERT(*item6_weak_ptr->remote_data == "sack1_item1");

    // test copy assignment operator =
    item3_weak_ptr = item1_weak_ptr;
    CPPUNIT_ASSERT_EQUAL(sack1->data_guard.size(), static_cast<std::size_t>(4));
    CPPUNIT_ASSERT(*item1_weak_ptr->remote_data == "sack1_item1");
    CPPUNIT_ASSERT(*item3_weak_ptr->remote_data == "sack1_item1");

    // test copy self-assignment operator =
    item3_weak_ptr = item3_weak_ptr;
    CPPUNIT_ASSERT_EQUAL(sack1->data_guard.size(), static_cast<std::size_t>(4));
    CPPUNIT_ASSERT(*item3_weak_ptr->remote_data == "sack1_item1");

    // test move assignment operator =
    item4_weak_ptr = std::move(item2_weak_ptr);
    CPPUNIT_ASSERT_EQUAL(sack1->data_guard.size(), static_cast<std::size_t>(4));
    CPPUNIT_ASSERT_THROW(static_cast<void>(*item2_weak_ptr->remote_data == "sack1_item2"), libdnf5::AssertionError);
    CPPUNIT_ASSERT(*item4_weak_ptr->remote_data == "sack1_item2");

    // test move self-assignment operator =
    item4_weak_ptr = std::move(item4_weak_ptr);
    CPPUNIT_ASSERT_EQUAL(sack1->data_guard.size(), static_cast<std::size_t>(4));
    CPPUNIT_ASSERT(*item4_weak_ptr->remote_data == "sack1_item2");

    // test operator ==
    // Each WeakPtr instance has its own data. However, the data may depend on the same remote data.
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr == item1_weak_ptr, true);
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr->remote_data == item1_weak_ptr->remote_data, true);
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr == item3_weak_ptr, false);
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr->remote_data == item3_weak_ptr->remote_data, true);
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr == item4_weak_ptr, false);
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr->remote_data == item4_weak_ptr->remote_data, false);

    // test operator !=
    // Each WeakPtr instance has its own data. However, the data may depend on the same remote data.
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr != item1_weak_ptr, false);
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr->remote_data != item1_weak_ptr->remote_data, false);
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr != item3_weak_ptr, true);
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr->remote_data != item3_weak_ptr->remote_data, false);
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr != item4_weak_ptr, true);
    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr->remote_data != item4_weak_ptr->remote_data, true);

    CPPUNIT_ASSERT_EQUAL(item1_weak_ptr != item4_weak_ptr, true);

    // test WeakPtrGuard::clear()
    // It must deregister and invalidate all registered weak pointers.
    CPPUNIT_ASSERT_EQUAL(sack1->data_guard.size(), static_cast<std::size_t>(4));
    CPPUNIT_ASSERT(*item1_weak_ptr->remote_data == "sack1_item1");
    CPPUNIT_ASSERT(*item3_weak_ptr->remote_data == "sack1_item1");
    CPPUNIT_ASSERT(*item4_weak_ptr->remote_data == "sack1_item2");
    CPPUNIT_ASSERT(*item6_weak_ptr->remote_data == "sack1_item1");
    sack1->data_guard.clear();
    CPPUNIT_ASSERT(sack1->data_guard.empty());
    CPPUNIT_ASSERT_EQUAL(sack1->data_guard.size(), static_cast<std::size_t>(0));
    CPPUNIT_ASSERT_THROW(static_cast<void>(*item1_weak_ptr->remote_data == "sack1_item1"), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(static_cast<void>(*item3_weak_ptr->remote_data == "sack1_item1"), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(static_cast<void>(*item4_weak_ptr->remote_data == "sack1_item1"), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(static_cast<void>(*item6_weak_ptr->remote_data == "sack1_item2"), libdnf5::AssertionError);
}
