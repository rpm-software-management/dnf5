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


#include "test_base.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/rpm/package_query.hpp"


CPPUNIT_TEST_SUITE_REGISTRATION(BaseTest);

void BaseTest::test_weak_ptr() {
    // Creates a new Base object
    auto base = std::make_unique<libdnf::Base>();

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
    CPPUNIT_ASSERT_THROW(vars->get_value("test_variable"), libdnf::InvalidPointerError);
    CPPUNIT_ASSERT_THROW(vars2->get_value("test_variable"), libdnf::InvalidPointerError);
}

void BaseTest::test_incorrect_workflow() {
    // Creates a new Base object
    auto base = std::make_unique<libdnf::Base>();

    // Base object is not fully initialized - not initialized by Base::setup()
    CPPUNIT_ASSERT_THROW(libdnf::rpm::PackageQuery(*base.get()), libdnf::AssertionError);

    base->setup();
    libdnf::rpm::PackageQuery(*base.get());
}
