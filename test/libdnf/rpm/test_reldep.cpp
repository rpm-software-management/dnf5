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

#include "test_reldep.hpp"

#include "libdnf/rpm/reldep.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(ReldepTest);


void ReldepTest::test_short_reldep() {
    libdnf::rpm::Reldep a(base, "labirinto.txt");
    CPPUNIT_ASSERT(std::string(a.get_name()) == "labirinto.txt");
    CPPUNIT_ASSERT(std::string(a.get_relation()) == "");
    CPPUNIT_ASSERT(std::string(a.get_version()) == "");
    CPPUNIT_ASSERT(a.to_string() == "labirinto.txt");

    libdnf::rpm::Reldep b(base, "labirinto.txt");
    CPPUNIT_ASSERT(a == b);
    CPPUNIT_ASSERT(a.get_id() == b.get_id());

    libdnf::rpm::Reldep c(base, "vagare");
    CPPUNIT_ASSERT(a != c);
}


void ReldepTest::test_full_reldep() {
    libdnf::rpm::Reldep a(base, "python3-labirinto = 4.2.0");
    CPPUNIT_ASSERT(std::string(a.get_name()) == "python3-labirinto");
    CPPUNIT_ASSERT(std::string(a.get_relation()) == " = ");
    CPPUNIT_ASSERT(std::string(a.get_version()) == "4.2.0");
    CPPUNIT_ASSERT(a.to_string() == "python3-labirinto = 4.2.0");

    libdnf::rpm::Reldep b(base, "python3-labirinto > 1.2.0");
    CPPUNIT_ASSERT(a != b);
}


void ReldepTest::test_rich_reldep() {
    libdnf::rpm::Reldep a(base, "(lab-list if labirinto.txt)");
    CPPUNIT_ASSERT(std::string(a.get_name()) == "lab-list");
    CPPUNIT_ASSERT(std::string(a.get_relation()) == " if ");
    CPPUNIT_ASSERT(std::string(a.get_version()) == "labirinto.txt");
    CPPUNIT_ASSERT(a.to_string() == "(lab-list if labirinto.txt)");

    libdnf::rpm::Reldep b(base, "(labirinto unless labirinto_c)");
    CPPUNIT_ASSERT(std::string(b.get_name()) == "labirinto");
    CPPUNIT_ASSERT(std::string(b.get_relation()) == " unless ");
    CPPUNIT_ASSERT(std::string(b.get_version()) == "labirinto_c");
    CPPUNIT_ASSERT(b.to_string() == "(labirinto unless labirinto_c)");

    CPPUNIT_ASSERT(a.get_id() != b.get_id());
    CPPUNIT_ASSERT(a != b);
}


void ReldepTest::test_invalid_reldep() {
    CPPUNIT_ASSERT_THROW(libdnf::rpm::Reldep a(base, "(lab-list if labirinto.txt"), libdnf::RuntimeError);
    CPPUNIT_ASSERT_THROW(libdnf::rpm::Reldep a(base, "labirinto = "), libdnf::RuntimeError);
}
