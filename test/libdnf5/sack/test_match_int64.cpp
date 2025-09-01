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


#include "test_match_int64.hpp"

#include <libdnf5/common/exception.hpp>
#include <libdnf5/common/sack/match_int64.hpp>


using namespace libdnf5::sack;


CPPUNIT_TEST_SUITE_REGISTRATION(SackMatchInt64Test);


void SackMatchInt64Test::test() {
    CPPUNIT_ASSERT(!match_int64(10, QueryCmp::EQ, 15));
    CPPUNIT_ASSERT(match_int64(10, QueryCmp::EQ, 10));
    CPPUNIT_ASSERT(!match_int64(10, QueryCmp::EQ, -10));

    CPPUNIT_ASSERT(match_int64(10, QueryCmp::NEQ, 15));
    CPPUNIT_ASSERT(!match_int64(10, QueryCmp::NEQ, 10));
    CPPUNIT_ASSERT(match_int64(10, QueryCmp::NEQ, -10));

    CPPUNIT_ASSERT(!match_int64(10, QueryCmp::GT, 15));
    CPPUNIT_ASSERT(!match_int64(10, QueryCmp::GT, 10));
    CPPUNIT_ASSERT(match_int64(10, QueryCmp::GT, -20));

    CPPUNIT_ASSERT(!match_int64(10, QueryCmp::GTE, 15));
    CPPUNIT_ASSERT(match_int64(10, QueryCmp::GTE, 10));
    CPPUNIT_ASSERT(match_int64(10, QueryCmp::GTE, -20));

    CPPUNIT_ASSERT(match_int64(10, QueryCmp::LT, 15));
    CPPUNIT_ASSERT(!match_int64(10, QueryCmp::LT, 10));
    CPPUNIT_ASSERT(!match_int64(10, QueryCmp::LT, -20));

    CPPUNIT_ASSERT(match_int64(10, QueryCmp::LTE, 15));
    CPPUNIT_ASSERT(match_int64(10, QueryCmp::LTE, 10));
    CPPUNIT_ASSERT(!match_int64(10, QueryCmp::LTE, -20));
}


void SackMatchInt64Test::test_invalid() {
    constexpr int64_t VALUE = 10;
    constexpr int64_t PATTERN = 5;

    CPPUNIT_ASSERT_THROW(match_int64(VALUE, QueryCmp::NOT, PATTERN), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(match_int64(VALUE, QueryCmp::ICASE, PATTERN), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(match_int64(VALUE, QueryCmp::NOT | QueryCmp::ICASE, PATTERN), libdnf5::AssertionError);

    CPPUNIT_ASSERT_THROW(match_int64(VALUE, QueryCmp::GLOB, PATTERN), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(match_int64(VALUE, QueryCmp::IGLOB, PATTERN), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(match_int64(VALUE, QueryCmp::REGEX, PATTERN), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(match_int64(VALUE, QueryCmp::IREGEX, PATTERN), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(match_int64(VALUE, QueryCmp::CONTAINS, PATTERN), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(match_int64(VALUE, QueryCmp::ICONTAINS, PATTERN), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(match_int64(VALUE, QueryCmp::STARTSWITH, PATTERN), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(match_int64(VALUE, QueryCmp::ISTARTSWITH, PATTERN), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(match_int64(VALUE, QueryCmp::ENDSWITH, PATTERN), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(match_int64(VALUE, QueryCmp::IENDSWITH, PATTERN), libdnf5::AssertionError);
}
