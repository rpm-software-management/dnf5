/*
Copyright (C) 2017-2020 Red Hat, Inc.

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


#ifndef TEST_LIBDNF_TRANSACTION_TEST_COMPS_GROUP_HPP
#define TEST_LIBDNF_TRANSACTION_TEST_COMPS_GROUP_HPP


#include "transaction_test_base.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class TransactionCompsGroupTest : public TransactionTestBase {
    CPPUNIT_TEST_SUITE(TransactionCompsGroupTest);
    CPPUNIT_TEST(test_save_load);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_save_load();
};


#endif  // TEST_LIBDNF_TRANSACTION_TEST_COMPS_GROUP_HPP
