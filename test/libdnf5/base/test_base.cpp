// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#include "test_base.hpp"

#include <libdnf5/base/base.hpp>
#include <libdnf5/rpm/package_query.hpp>


CPPUNIT_TEST_SUITE_REGISTRATION(BaseTest);

void BaseTest::test_weak_ptr() {
    // Creates a new Base object
    auto base = get_preconfigured_base();

    // Gets a WeakPtr pointing to Vars in the Base object
    auto vars = base->get_vars();

    // Creates a copy of WeakPtr
    auto vars2 = vars;

    // Base is valid -> WeakPtr is valid. Sets "test_variable" using WeakPtr vars.
    vars->set("test_variable", "value1");

    // Base is valid -> WeakPtr is valid. Gets value of "test_variable" using copy of WeakPtr vars2.
    CPPUNIT_ASSERT_EQUAL(std::string("value1"), vars2->get_value("test_variable"));

    // Invalidates Base object
    base.reset();

    // Base object is invalid. -> Both WeakPtr are invalid. The code must throw an exception.
    CPPUNIT_ASSERT_THROW(vars->get_value("test_variable"), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(vars2->get_value("test_variable"), libdnf5::AssertionError);
}

void BaseTest::test_missing_setup() {
    // Creates a new Base object
    auto base = get_preconfigured_base();

    // Base object is not fully initialized - not initialized by Base::setup()
    CPPUNIT_ASSERT_THROW(libdnf5::rpm::PackageQuery(*base.get()), libdnf5::UserAssertionError);

    base->setup();
    libdnf5::rpm::PackageQuery(*base.get());
}

void BaseTest::test_repeated_setup() {
    // Creates a new Base object
    auto base = get_preconfigured_base();

    // Initialize the Base
    base->setup();

    // Base was already initialized
    CPPUNIT_ASSERT_THROW(base->setup(), libdnf5::UserAssertionError);
}

void BaseTest::test_unlock_not_locked() {
    // Creates a new Base object
    auto base = get_preconfigured_base();

    // Base::unlock() called on unlocked Base instance
    CPPUNIT_ASSERT_THROW(base->unlock(), libdnf5::UserAssertionError);

    // Lock the Base first
    base->lock();

    // Unlocking should work now
    base->unlock();
}
