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

#ifndef LIBDNF5_TEST_RPM_SOLV_ID_QUEUE_HPP
#define LIBDNF5_TEST_RPM_SOLV_ID_QUEUE_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class IdQueueTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(IdQueueTest);

#ifndef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_push_back);
    CPPUNIT_TEST(test_operators);
    CPPUNIT_TEST(test_iterator_empty);
    CPPUNIT_TEST(test_iterator_full);
#endif

#ifdef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_iterator_performance);
#endif

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_push_back();
    void test_operators();
    void test_iterator_empty();
    void test_iterator_full();
    void test_iterator_performance();

private:
};

#endif  // LIBDNF5_TEST_RPM_SOLV_ID_QUEUE_HPP
