/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "test_impl_ptr.hpp"

#include <libdnf5/common/impl_ptr.hpp>

#include <type_traits>
#include <utility>


CPPUNIT_TEST_SUITE_REGISTRATION(ImplPtrTest);

namespace {

// Test class with instance counter. It is non move constructible, non move assignable, and non swappable.
class CTest {
public:
    CTest() noexcept { ++instance_counter; }
    CTest(int a) : a(a) { ++instance_counter; }
    CTest(const CTest & src) noexcept : a(src.a) { ++instance_counter; }
    CTest(CTest && src) = delete;

    CTest & operator=(const CTest & src) noexcept {
        a = src.a;
        return *this;
    }

    CTest & operator=(CTest && src) = delete;

    ~CTest() { --instance_counter; }

    // Returns the number of existing class instances.
    static int get_instance_counter() noexcept { return instance_counter; }

    int get_a() const noexcept { return a; }
    void set_a(int value) noexcept { a = value; }

private:
    int a{0};
    static int instance_counter;
};

int CTest::instance_counter = 0;

}  // namespace


static_assert(std::is_nothrow_default_constructible_v<libdnf5::ImplPtr<CTest>>);
static_assert(std::is_nothrow_default_constructible_v<libdnf5::ImplPtr<CTest>>);
static_assert(std::is_copy_constructible_v<libdnf5::ImplPtr<CTest>>);
static_assert(std::is_copy_assignable_v<libdnf5::ImplPtr<CTest>>);

// `ImplPtr` is_nothrow_move_constructible, is_nothrow_move_assignable, and nothrow_swappable, even if CTest is not.
static_assert(!std::is_move_constructible_v<CTest>);
static_assert(!std::is_move_assignable_v<CTest>);
static_assert(!std::is_swappable_v<CTest>);
static_assert(std::is_nothrow_move_constructible_v<libdnf5::ImplPtr<CTest>>);
static_assert(std::is_nothrow_move_assignable_v<libdnf5::ImplPtr<CTest>>);
static_assert(std::is_nothrow_swappable_v<libdnf5::ImplPtr<CTest>>);


void ImplPtrTest::test_default_constructor() {
    // Tests an empty constructor. `p_impl` is` nullptr`. No new CTest instance is created.
    libdnf5::ImplPtr<CTest> empty_object;
    CPPUNIT_ASSERT(nullptr == empty_object.get());
    CPPUNIT_ASSERT_EQUAL(0, CTest::get_instance_counter());
}

void ImplPtrTest::test_constructor_from_pointer() {
    {
        // Tests constructor that takes ownership of existing `CTtest` instance.
        libdnf5::ImplPtr<CTest> object(new CTest);
        CPPUNIT_ASSERT(nullptr != object.get());
        CPPUNIT_ASSERT_EQUAL(1, CTest::get_instance_counter());
    }

    // `ImplPtr` was the owner of the` CTest` instance and destroyed it.
    CPPUNIT_ASSERT_EQUAL(0, CTest::get_instance_counter());
}

void ImplPtrTest::test_access_to_managed_object() {
    {
        // Constructs mutable (non-constant) object.
        libdnf5::ImplPtr<CTest> object(new CTest(10));

        // Tests the `T * operator->()`.
        static_assert(!std::is_const_v<std::remove_pointer_t<decltype(object.operator->())>>);
        CPPUNIT_ASSERT_EQUAL(10, object->get_a());
        object->set_a(0);
        CPPUNIT_ASSERT_EQUAL(0, object->get_a());

        // Tests the `T & operator*()`.
        static_assert(!std::is_const_v<std::remove_reference_t<decltype(object.operator*())>>);
        CPPUNIT_ASSERT_EQUAL(0, (*object).get_a());
        (*object).set_a(10);
        CPPUNIT_ASSERT_EQUAL(10, (*object).get_a());

        // Tests the `T * get()`.
        static_assert(!std::is_const_v<std::remove_pointer_t<decltype(object.get())>>);
        CPPUNIT_ASSERT_EQUAL(10, object.get()->get_a());
        object.get()->set_a(0);
        CPPUNIT_ASSERT_EQUAL(0, object.get()->get_a());
    }

    // Tests that all instances of the `CTest` class are destroyed. No leaks.
    CPPUNIT_ASSERT_EQUAL(0, CTest::get_instance_counter());
}

void ImplPtrTest::test_const_access_to_managed_object() {
    {
        // Constructs constant object.
        const libdnf5::ImplPtr<CTest> const_object(new CTest(10));

        // Tests the `const T * operator->() const`
        static_assert(std::is_const_v<std::remove_pointer_t<decltype(const_object.operator->())>>);
        CPPUNIT_ASSERT_EQUAL(10, const_object->get_a());

        // Tests the `const T & operator*() const`.
        static_assert(std::is_const_v<std::remove_reference_t<decltype(const_object.operator*())>>);
        CPPUNIT_ASSERT_EQUAL(10, (*const_object).get_a());

        // Tests the `const T * get() const`.
        static_assert(std::is_const_v<std::remove_pointer_t<decltype(const_object.get())>>);
        CPPUNIT_ASSERT_EQUAL(10, const_object.get()->get_a());
    }

    // Tests that all instances of the `CTest` class are destroyed. No leaks.
    CPPUNIT_ASSERT_EQUAL(0, CTest::get_instance_counter());
}

void ImplPtrTest::test_copy_constructor() {
    {
        libdnf5::ImplPtr<CTest> src_object(new CTest(10));
        CPPUNIT_ASSERT_EQUAL(1, CTest::get_instance_counter());

        // Tests the copy constructor. The source object remains unchanged.
        // A new `CTest` instance is created.
        libdnf5::ImplPtr<CTest> new_object(src_object);
        CPPUNIT_ASSERT_EQUAL(2, CTest::get_instance_counter());
        CPPUNIT_ASSERT_EQUAL(10, new_object->get_a());
        CPPUNIT_ASSERT(nullptr != src_object.get());
        CPPUNIT_ASSERT_EQUAL(10, src_object->get_a());

        // Tests the copy constructor. The empty source object.
        // No new `CTest` instance is created.
        libdnf5::ImplPtr<CTest> empty_object;
        CPPUNIT_ASSERT_EQUAL(2, CTest::get_instance_counter());
        libdnf5::ImplPtr<CTest> new_empty_object_copy(empty_object);
        CPPUNIT_ASSERT_EQUAL(2, CTest::get_instance_counter());
        CPPUNIT_ASSERT(nullptr == empty_object.get());
        CPPUNIT_ASSERT(nullptr == new_empty_object_copy.get());
    }

    // Tests that all instances of the `CTest` class are destroyed. No leaks.
    CPPUNIT_ASSERT_EQUAL(0, CTest::get_instance_counter());
}

void ImplPtrTest::test_move_constructor() {
    {
        libdnf5::ImplPtr<CTest> src_object(new CTest(10));
        CPPUNIT_ASSERT_EQUAL(1, CTest::get_instance_counter());

        // Tests the move constructor. Ownership is transferred from the source object to `*this`.
        // `p_impl` in the source object becomes `nullptr`.
        // No new `CTest` instances are created.
        libdnf5::ImplPtr<CTest> new_object(std::move(src_object));
        CPPUNIT_ASSERT_EQUAL(1, CTest::get_instance_counter());
        CPPUNIT_ASSERT_EQUAL(10, new_object->get_a());
        CPPUNIT_ASSERT(nullptr == src_object.get());

        // Tests the move constructor. The empty source object.
        // No new `CTest` instances are created.
        libdnf5::ImplPtr<CTest> empty_object;
        libdnf5::ImplPtr<CTest> new_object2(std::move(empty_object));
        CPPUNIT_ASSERT_EQUAL(1, CTest::get_instance_counter());
        CPPUNIT_ASSERT(nullptr == new_object2.get());
        CPPUNIT_ASSERT(nullptr == src_object.get());
    }

    // Tests that all instances of the `CTest` class are destroyed. No leaks.
    CPPUNIT_ASSERT_EQUAL(0, CTest::get_instance_counter());
}

void ImplPtrTest::test_copy_assignment() {
    {
        libdnf5::ImplPtr<CTest> src_object(new CTest(10));
        libdnf5::ImplPtr<CTest> dst_object(new CTest(5));
        CPPUNIT_ASSERT_EQUAL(2, CTest::get_instance_counter());

        // Tests the copy assignment operator. The source object remains unchanged.
        // The value of the `CTest` instance in the source is copied to the CTest instance in the destination.
        dst_object = src_object;
        CPPUNIT_ASSERT_EQUAL(2, CTest::get_instance_counter());
        CPPUNIT_ASSERT_EQUAL(10, dst_object->get_a());
        CPPUNIT_ASSERT(nullptr != src_object.get());
        CPPUNIT_ASSERT_EQUAL(10, src_object->get_a());

        // Tests the copy assignment operator - self-assignment.
        // The object must remain unchanged.
        dst_object = dst_object;
        CPPUNIT_ASSERT_EQUAL(2, CTest::get_instance_counter());
        CPPUNIT_ASSERT_EQUAL(10, dst_object->get_a());

        // Tests the copy assignment to empty (moved from) object. The source object remains unchanged.
        // A new `CTest` instance is created.
        libdnf5::ImplPtr<CTest> empty_dst_object;
        CPPUNIT_ASSERT(nullptr == empty_dst_object.get());
        empty_dst_object = dst_object;
        CPPUNIT_ASSERT_EQUAL(3, CTest::get_instance_counter());
        CPPUNIT_ASSERT_EQUAL(10, empty_dst_object->get_a());
        CPPUNIT_ASSERT_EQUAL(10, dst_object->get_a());

        // Tests the copy assignment from empty to non empty object.
        // The old `CTest` instance in the destination is destroyed.
        libdnf5::ImplPtr<CTest> empty_src_object;
        CPPUNIT_ASSERT_EQUAL(3, CTest::get_instance_counter());
        CPPUNIT_ASSERT(nullptr == empty_src_object.get());
        CPPUNIT_ASSERT(nullptr != dst_object.get());
        dst_object = empty_src_object;
        CPPUNIT_ASSERT(nullptr == empty_src_object.get());
        CPPUNIT_ASSERT(nullptr == dst_object.get());
        CPPUNIT_ASSERT_EQUAL(2, CTest::get_instance_counter());
    }

    // Tests that all instances of the `CTest` class are destroyed. No leaks.
    CPPUNIT_ASSERT_EQUAL(0, CTest::get_instance_counter());
}

void ImplPtrTest::test_move_assignment() {
    {
        libdnf5::ImplPtr<CTest> src_object(new CTest(10));
        libdnf5::ImplPtr<CTest> dst_object(new CTest(5));
        CPPUNIT_ASSERT_EQUAL(2, CTest::get_instance_counter());

        // Tests the move assignment operator.
        // `CTest` instance ownership is transferred from the source object to the destination.
        // The old `CTest` instance in the destination is destroyed.
        // `p_impl` in the original object becomes nullptr.
        dst_object = std::move(src_object);
        CPPUNIT_ASSERT_EQUAL(1, CTest::get_instance_counter());
        CPPUNIT_ASSERT_EQUAL(10, dst_object->get_a());
        CPPUNIT_ASSERT(nullptr == src_object.get());

        // Tests the move assignment operator - self-assignment.
        // The object must remain unchanged.
        dst_object = std::move(dst_object);
        CPPUNIT_ASSERT_EQUAL(1, CTest::get_instance_counter());
        CPPUNIT_ASSERT_EQUAL(10, dst_object->get_a());

        // Tests the move assignment to empty (moved from) object.
        // `CTest` instance ownership is transferred from the source object to the destination.
        // `p_impl` in the original object becomes nullptr.
        libdnf5::ImplPtr<CTest> empty_object;
        CPPUNIT_ASSERT(nullptr == empty_object.get());
        empty_object = std::move(dst_object);
        CPPUNIT_ASSERT_EQUAL(1, CTest::get_instance_counter());
        CPPUNIT_ASSERT_EQUAL(10, empty_object->get_a());
        CPPUNIT_ASSERT(nullptr == dst_object.get());

        // Tests the move assignment from empty to non empty object.
        // The old `CTest` instance in the destination is destroyed.
        libdnf5::ImplPtr<CTest> empty_src_object;
        libdnf5::ImplPtr<CTest> non_empty_dst_object(new CTest(5));
        CPPUNIT_ASSERT_EQUAL(2, CTest::get_instance_counter());
        non_empty_dst_object = std::move(empty_src_object);
        CPPUNIT_ASSERT(nullptr == empty_src_object.get());
        CPPUNIT_ASSERT(nullptr == non_empty_dst_object.get());
        CPPUNIT_ASSERT_EQUAL(1, CTest::get_instance_counter());
    }

    // Tests that all instances of the `CTest` class are destroyed. No leaks.
    CPPUNIT_ASSERT_EQUAL(0, CTest::get_instance_counter());
}
