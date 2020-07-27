/*
Copyright (C) 2020 Red Hat, Inc.

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

#include "test_reldep.hpp"

#include "libdnf/rpm/reldep.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(ReldepTest);


void ReldepTest::setUp() {
    base = std::make_unique<libdnf::Base>();
    sack = std::make_unique<libdnf::rpm::SolvSack>(*base);
}


void ReldepTest::test_short_reldep() {
    libdnf::rpm::Reldep a(sack.get(), "labirinto.txt");
    CPPUNIT_ASSERT(std::string(a.get_name()) == "labirinto.txt");
    CPPUNIT_ASSERT(std::string(a.get_relation()) == "");
    CPPUNIT_ASSERT(std::string(a.get_version()) == "");
    CPPUNIT_ASSERT(a.to_string() == "labirinto.txt");

    libdnf::rpm::Reldep b(sack.get(), "labirinto.txt");
    CPPUNIT_ASSERT(a == b);
    CPPUNIT_ASSERT(a.get_id() == b.get_id());

    libdnf::rpm::Reldep c(sack.get(), "vagare");
    CPPUNIT_ASSERT(a != c);
}


void ReldepTest::test_full_reldep() {
    libdnf::rpm::Reldep a(sack.get(), "python3-labirinto = 4.2.0");
    CPPUNIT_ASSERT(std::string(a.get_name()) == "python3-labirinto");
    CPPUNIT_ASSERT(std::string(a.get_relation()) == " = ");
    CPPUNIT_ASSERT(std::string(a.get_version()) == "4.2.0");
    CPPUNIT_ASSERT(a.to_string() == "python3-labirinto = 4.2.0");

    libdnf::rpm::Reldep b(sack.get(), "python3-labirinto > 1.2.0");
    CPPUNIT_ASSERT(a != b);
}


void ReldepTest::test_rich_reldep() {
    libdnf::rpm::Reldep a(sack.get(), "(lab-list if labirinto.txt)");
    CPPUNIT_ASSERT(std::string(a.get_name()) == "lab-list");
    CPPUNIT_ASSERT(std::string(a.get_relation()) == " if ");
    CPPUNIT_ASSERT(std::string(a.get_version()) == "labirinto.txt");
    CPPUNIT_ASSERT(a.to_string() == "(lab-list if labirinto.txt)");

    libdnf::rpm::Reldep b(sack.get(), "(labirinto unless labirinto_c)");
    CPPUNIT_ASSERT(std::string(b.get_name()) == "labirinto");
    CPPUNIT_ASSERT(std::string(b.get_relation()) == " unless ");
    CPPUNIT_ASSERT(std::string(b.get_version()) == "labirinto_c");
    CPPUNIT_ASSERT(b.to_string() == "(labirinto unless labirinto_c)");

    CPPUNIT_ASSERT(a.get_id() != b.get_id());
    CPPUNIT_ASSERT(a != b);
}


void ReldepTest::test_invalid_reldep() {
    CPPUNIT_ASSERT_THROW(libdnf::rpm::Reldep a(sack.get(), "(lab-list if labirinto.txt"), std::runtime_error);
    CPPUNIT_ASSERT_THROW(libdnf::rpm::Reldep a(sack.get(), "labirinto = "), std::runtime_error);
}
