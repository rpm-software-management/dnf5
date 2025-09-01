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


#include "test_reldep_parser.hpp"

#include "solv/reldep_parser.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(ReldepParserTest);


void ReldepParserTest::setUp() {}


void ReldepParserTest::tearDown() {}


// Ensure that both Ids from libsolv nad libdnf represent the same type
void ReldepParserTest::test_parser() {
    std::string reldep_str("dnf = 34.66.7-67.4");
    libdnf5::solv::ReldepParser dep_parser;
    CPPUNIT_ASSERT_MESSAGE("Parse 'dnf = 34.66.7-67.4'", dep_parser.parse(reldep_str));
    CPPUNIT_ASSERT_MESSAGE("Parse 'dnf = 34.66.7-67.4'", dep_parser.get_name() == "dnf");
    CPPUNIT_ASSERT_MESSAGE(
        "Parse 'dnf = 34.66.7-67.4'", dep_parser.get_cmp_type() == libdnf5::rpm::Reldep::CmpType::EQ);
    CPPUNIT_ASSERT_MESSAGE("Parse 'dnf = 34.66.7-67.4'", dep_parser.get_evr() == "34.66.7-67.4");

    reldep_str = "dnf == 34.66.7-67.4";

    CPPUNIT_ASSERT_MESSAGE(
        "Parse 'dnf == 34.66.7-67.4' with unsupported operator '==' must fail", !dep_parser.parse(reldep_str));

    reldep_str = "libdnf5 <= h";
    CPPUNIT_ASSERT_MESSAGE("Parse 'libdnf5 <= h'", dep_parser.parse(reldep_str));
    CPPUNIT_ASSERT_MESSAGE("Parse 'libdnf5 <= h'", dep_parser.get_name() == "libdnf5");
    CPPUNIT_ASSERT_MESSAGE("Parse 'libdnf5 <= h'", dep_parser.get_cmp_type() == libdnf5::rpm::Reldep::CmpType::LTE);
    CPPUNIT_ASSERT_MESSAGE("Parse 'libdnf5 <= h'", dep_parser.get_evr() == "h");

    reldep_str = "dnf < 34.66.7-67.4";
    CPPUNIT_ASSERT_MESSAGE("Parse 'dnf < 34.66.7-67.4'", dep_parser.parse(reldep_str));
    CPPUNIT_ASSERT_MESSAGE("Parse 'dnf < 34.66.7-67.4'", dep_parser.get_name() == "dnf");
    CPPUNIT_ASSERT_MESSAGE(
        "Parse 'dnf < 34.66.7-67.4'", dep_parser.get_cmp_type() == libdnf5::rpm::Reldep::CmpType::LT);
    CPPUNIT_ASSERT_MESSAGE("Parse 'dnf < 34.66.7-67.4'", dep_parser.get_evr() == "34.66.7-67.4");

    reldep_str = "dnf > 34.66.7-67.4";
    CPPUNIT_ASSERT_MESSAGE("Parse 'dnf > 34.66.7-67.4'", dep_parser.parse(reldep_str));
    CPPUNIT_ASSERT_MESSAGE("Parse 'dnf > 34.66.7-67.4'", dep_parser.get_name() == "dnf");
    CPPUNIT_ASSERT_MESSAGE(
        "Parse 'dnf > 34.66.7-67.4'", dep_parser.get_cmp_type() == libdnf5::rpm::Reldep::CmpType::GT);
    CPPUNIT_ASSERT_MESSAGE("Parse 'dnf > 34.66.7-67.4'", dep_parser.get_evr() == "34.66.7-67.4");

    reldep_str = "dnf >= 34.66.7-67.4";
    CPPUNIT_ASSERT_MESSAGE("Parse 'dnf >= 34.66.7-67.4'", dep_parser.parse(reldep_str));
    CPPUNIT_ASSERT_MESSAGE("Parse 'dnf >= 34.66.7-67.4'", dep_parser.get_name() == "dnf");
    CPPUNIT_ASSERT_MESSAGE(
        "Parse 'dnf >= 34.66.7-67.4'", dep_parser.get_cmp_type() == libdnf5::rpm::Reldep::CmpType::GTE);
    CPPUNIT_ASSERT_MESSAGE("Parse 'dnf >= 34.66.7-67.4'", dep_parser.get_evr() == "34.66.7-67.4");

    reldep_str = "python3-astroid <> 2.3.3-2.gitace7b29.fc31";
    CPPUNIT_ASSERT_MESSAGE(
        "Parse 'python3-astroid <> 2.3.3-2.gitace7b29.fc31' with unsupported "
        "operator '==' must fail",
        !dep_parser.parse(reldep_str));

    reldep_str = "space.txt";
    CPPUNIT_ASSERT_MESSAGE("Parse 'space.txt'", dep_parser.parse(reldep_str));
    CPPUNIT_ASSERT_MESSAGE("Parse 'space.txt'", dep_parser.get_name() == "space.txt");
    CPPUNIT_ASSERT_MESSAGE("Parse 'space.txt'", dep_parser.get_cmp_type() == libdnf5::rpm::Reldep::CmpType::NONE);
    CPPUNIT_ASSERT_MESSAGE("Parse 'space.txt'", dep_parser.get_evr() == "");

    reldep_str = "python3-astroid =";
    CPPUNIT_ASSERT_MESSAGE("Parse not valid reldep 'python3-astroid =' must fail", !dep_parser.parse(reldep_str));

    reldep_str = "python3-astroid 56.34";
    CPPUNIT_ASSERT_MESSAGE("Parse not valid reldep 'python3-astroid 56.34' must fail", !dep_parser.parse(reldep_str));

    reldep_str = " python3-astroid";
    CPPUNIT_ASSERT_MESSAGE(
        "Parse not valid reldep ' python3-astroid' with space at beginning "
        "must fail",
        !dep_parser.parse(reldep_str));

    reldep_str = "/file/provide/space.txt";
    CPPUNIT_ASSERT_MESSAGE("Parse '/file/provide/space.txt'", dep_parser.parse(reldep_str));
    CPPUNIT_ASSERT_MESSAGE("Parse '/file/provide/space.txt'", dep_parser.get_name() == "/file/provide/space.txt");
    CPPUNIT_ASSERT_MESSAGE(
        "Parse '/file/provide/space.txt'", dep_parser.get_cmp_type() == libdnf5::rpm::Reldep::CmpType::NONE);
    CPPUNIT_ASSERT_MESSAGE("Parse '/file/provide/space.txt'", dep_parser.get_evr() == "");

    reldep_str = "python3-astroid > sdfsdf asdad";
    CPPUNIT_ASSERT_MESSAGE(
        "Parse not valid reldep 'python3-astroid > sdfsdf asdad' with 4 elements"
        "must fail",
        !dep_parser.parse(reldep_str));
}
