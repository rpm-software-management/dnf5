/*
Copyright (C) 2021 Red Hat, Inc.

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


#ifndef LIBDNF_TEST_COMPS_GROUP_HPP
#define LIBDNF_TEST_COMPS_GROUP_HPP


#include "libdnf/base/base.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class CompsGroupTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(CompsGroupTest);
    CPPUNIT_TEST(test_load);
    CPPUNIT_TEST(test_load_defaults);
    CPPUNIT_TEST(test_merge);
    CPPUNIT_TEST(test_merge_with_empty);
    CPPUNIT_TEST(test_merge_empty_with_nonempty);
    CPPUNIT_TEST(test_dump_and_load);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_load();
    void test_load_defaults();
    void test_merge();
    void test_merge_with_empty();
    void test_merge_empty_with_nonempty();
    void test_dump_and_load();

protected:
    std::unique_ptr<libdnf::Base> base;
};

#endif
