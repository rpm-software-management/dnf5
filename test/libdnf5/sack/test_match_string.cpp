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


#include "test_match_string.hpp"

#include <libdnf5/common/exception.hpp>
#include <libdnf5/common/sack/match_string.hpp>


using namespace libdnf5::sack;


CPPUNIT_TEST_SUITE_REGISTRATION(SackMatchStringTest);


void SackMatchStringTest::test() {
    const std::string value{"AbCdEfGhIjKlMnOp"};

    const std::string test_patterns[]{
        "AbCdEfGhIjKlMnOp",
        "ABcdEfGhIjKlMNop",
        "AbCdEf",
        "ABcdEf",
        "KlMnOp",
        "KlMNoP",
        "EfGh",
        "EFgh",
        "A[a-d]Cd*Gh*Ij?lMnOp",
        "A[A-D]Cd*Gh*Ij?lMNoP",
        "A[bdCE]+fGhIj.lMnOp",
        "A[b-e]+fGhIj.lMNop"};

    struct OperatorTests {
        // tested operator
        QueryCmp cmp;
        union {
            // expected results by name
            struct {
                bool exact;
                bool iexact;
                bool start;
                bool istart;
                bool end;
                bool iend;
                bool contain;
                bool icontain;
                bool glob;
                bool iglob;
                bool reg;
                bool ireg;
            };
            // expected results as array
            bool results[12];
        };
    };

    const OperatorTests tests[]{
        {.cmp = QueryCmp::EXACT,
         .exact = true,
         .iexact = false,
         .start = false,
         .istart = false,
         .end = false,
         .iend = false,
         .contain = false,
         .icontain = false,
         .glob = false,
         .iglob = false,
         .reg = false,
         .ireg = false},
        {.cmp = QueryCmp::IEXACT,
         .exact = true,
         .iexact = true,
         .start = false,
         .istart = false,
         .end = false,
         .iend = false,
         .contain = false,
         .icontain = false,
         .glob = false,
         .iglob = false,
         .reg = false,
         .ireg = false},
        {.cmp = QueryCmp::GLOB,
         .exact = true,
         .iexact = false,
         .start = false,
         .istart = false,
         .end = false,
         .iend = false,
         .contain = false,
         .icontain = false,
         .glob = true,
         .iglob = false,
         .reg = false,
         .ireg = false},
        {.cmp = QueryCmp::IGLOB,
         .exact = true,
         .iexact = true,
         .start = false,
         .istart = false,
         .end = false,
         .iend = false,
         .contain = false,
         .icontain = false,
         .glob = true,
         .iglob = true,
         .reg = false,
         .ireg = false},
        {.cmp = QueryCmp::REGEX,
         .exact = true,
         .iexact = false,
         .start = false,
         .istart = false,
         .end = false,
         .iend = false,
         .contain = false,
         .icontain = false,
         .glob = false,
         .iglob = false,
         .reg = true,
         .ireg = false},
        {.cmp = QueryCmp::IREGEX,
         .exact = true,
         .iexact = true,
         .start = false,
         .istart = false,
         .end = false,
         .iend = false,
         .contain = false,
         .icontain = false,
         .glob = false,
         .iglob = false,
         .reg = true,
         .ireg = true},
        {.cmp = QueryCmp::CONTAINS,
         .exact = true,
         .iexact = false,
         .start = true,
         .istart = false,
         .end = true,
         .iend = false,
         .contain = true,
         .icontain = false,
         .glob = false,
         .iglob = false,
         .reg = false,
         .ireg = false},
        {.cmp = QueryCmp::ICONTAINS,
         .exact = true,
         .iexact = true,
         .start = true,
         .istart = true,
         .end = true,
         .iend = true,
         .contain = true,
         .icontain = true,
         .glob = false,
         .iglob = false,
         .reg = false,
         .ireg = false},
        {.cmp = QueryCmp::STARTSWITH,
         .exact = true,
         .iexact = false,
         .start = true,
         .istart = false,
         .end = false,
         .iend = false,
         .contain = false,
         .icontain = false,
         .glob = false,
         .iglob = false,
         .reg = false,
         .ireg = false},
        {.cmp = QueryCmp::ISTARTSWITH,
         .exact = true,
         .iexact = true,
         .start = true,
         .istart = true,
         .end = false,
         .iend = false,
         .contain = false,
         .icontain = false,
         .glob = false,
         .iglob = false,
         .reg = false,
         .ireg = false},
        {.cmp = QueryCmp::ENDSWITH,
         .exact = true,
         .iexact = false,
         .start = false,
         .istart = false,
         .end = true,
         .iend = false,
         .contain = false,
         .icontain = false,
         .glob = false,
         .iglob = false,
         .reg = false,
         .ireg = false},
        {.cmp = QueryCmp::IENDSWITH,
         .exact = true,
         .iexact = true,
         .start = false,
         .istart = false,
         .end = true,
         .iend = true,
         .contain = false,
         .icontain = false,
         .glob = false,
         .iglob = false,
         .reg = false,
         .ireg = false}};

    for (const auto & operator_tests : tests) {
        for (std::size_t i = 0; i < sizeof(test_patterns) / sizeof(test_patterns[0]); ++i) {
            CPPUNIT_ASSERT_EQUAL(operator_tests.results[i], match_string(value, operator_tests.cmp, test_patterns[i]));
        }

        // Tests with the NOT modifier/flag.
        for (std::size_t i = 0; i < sizeof(test_patterns) / sizeof(test_patterns[0]); ++i) {
            CPPUNIT_ASSERT_EQUAL(
                !operator_tests.results[i], match_string(value, operator_tests.cmp | QueryCmp::NOT, test_patterns[i]));
        }
    }
}


void SackMatchStringTest::test_invalid() {
    const std::string VALUE = "VALUE";
    const std::string PATTERN = "PATTERN";

    CPPUNIT_ASSERT_THROW(match_string(VALUE, QueryCmp::NOT, PATTERN), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(match_string(VALUE, QueryCmp::ICASE, PATTERN), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(match_string("VALUE", QueryCmp::NOT | QueryCmp::ICASE, PATTERN), libdnf5::AssertionError);

    CPPUNIT_ASSERT_THROW(match_string("VALUE", QueryCmp::ISNULL, PATTERN), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(match_string("VALUE", QueryCmp::GT, PATTERN), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(match_string("VALUE", QueryCmp::GTE, PATTERN), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(match_string("VALUE", QueryCmp::LT, PATTERN), libdnf5::AssertionError);
    CPPUNIT_ASSERT_THROW(match_string("VALUE", QueryCmp::LTE, PATTERN), libdnf5::AssertionError);
}
