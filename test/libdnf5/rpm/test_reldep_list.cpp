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

#include "test_reldep_list.hpp"

#include "../shared/utils.hpp"

#include <libdnf5/rpm/reldep_list.hpp>


using libdnf5::rpm::Reldep;


CPPUNIT_TEST_SUITE_REGISTRATION(ReldepListTest);


void ReldepListTest::test_get() {
    libdnf5::rpm::ReldepList list1(base);
    libdnf5::rpm::Reldep a(base, "python3-labirinto = 4.2.0");
    libdnf5::rpm::Reldep b(base, "(lab-list if labirinto.txt)");
    libdnf5::rpm::Reldep c(base, "(labirinto unless labirinto_c)");

    list1.add(a);
    list1.add(b);
    list1.add(c);

    CPPUNIT_ASSERT(list1.get(0) == a);
    CPPUNIT_ASSERT(list1.get(1).to_string() == "(lab-list if labirinto.txt)");
    CPPUNIT_ASSERT(list1.get_id(2).id == c.get_id().id);
}


void ReldepListTest::test_add() {
    libdnf5::rpm::ReldepList list1(base);
    libdnf5::rpm::Reldep a(base, "python3-labirinto = 4.2.0");
    libdnf5::rpm::Reldep b(base, "(lab-list if labirinto.txt)");

    list1.add(a);
    list1.add(b.get_id());
    list1.add_reldep("delgado > 1.2");
    CPPUNIT_ASSERT(list1.size() == 3);
}


void ReldepListTest::test_size() {
    libdnf5::rpm::ReldepList list1(base);
    CPPUNIT_ASSERT(list1.size() == 0);

    libdnf5::rpm::Reldep a(base, "python3-labirinto = 4.2.0");
    libdnf5::rpm::Reldep b(base, "(lab-list if labirinto.txt)");
    list1.add(a);
    list1.add(b);
    CPPUNIT_ASSERT(list1.size() == 2);
}


void ReldepListTest::test_compare() {
    libdnf5::rpm::ReldepList list1(base);
    libdnf5::rpm::ReldepList list2(base);
    libdnf5::rpm::Reldep a(base, "python3-labirinto = 4.2.0");
    libdnf5::rpm::Reldep b(base, "(lab-list if labirinto.txt)");

    CPPUNIT_ASSERT(list1 == list2);

    list1.add(a);
    CPPUNIT_ASSERT(list1 != list2);

    list2.add(a);
    list1.add(b);
    list2.add(b);
    CPPUNIT_ASSERT(list1 == list2);
}


void ReldepListTest::test_append() {
    libdnf5::rpm::Reldep a(base, "python3-labirinto = 4.2.0");
    libdnf5::rpm::Reldep b(base, "(lab-list if labirinto.txt)");
    libdnf5::rpm::Reldep c(base, "(labirinto unless labirinto_c)");
    libdnf5::rpm::Reldep d(base, "labirinto.txt");

    libdnf5::rpm::ReldepList list1(base);
    list1.add(a);
    list1.add(b);
    list1.add_reldep("delgado > 1.2");


    libdnf5::rpm::ReldepList list2(base);
    list2.add(c);
    list2.add(d);

    list1.append(list2);
    CPPUNIT_ASSERT(list1.size() == 5);
    CPPUNIT_ASSERT(list1.get(0) == a);
    CPPUNIT_ASSERT(list1.get(1) == b);
    CPPUNIT_ASSERT(list1.get(2).to_string() == "delgado > 1.2");
    CPPUNIT_ASSERT(list1.get(3) == c);
    CPPUNIT_ASSERT(list1.get(4) == d);
}

void ReldepListTest::test_iterator() {
    libdnf5::rpm::Reldep a(base, "python3-labirinto = 4.2.0");
    libdnf5::rpm::Reldep b(base, "(lab-list if labirinto.txt)");
    libdnf5::rpm::Reldep c(base, "(labirinto unless labirinto_c)");
    libdnf5::rpm::Reldep d(base, "labirinto.txt");
    std::vector<libdnf5::rpm::Reldep> expected;
    libdnf5::rpm::ReldepList list(base);

    expected.push_back(a);
    list.add(a);
    expected.push_back(b);
    list.add(b);
    expected.push_back(c);
    list.add(c);
    expected.push_back(d);
    list.add(d);

    // check if begin() points to the first Reldep
    auto it1 = list.begin();
    CPPUNIT_ASSERT(*it1 == a);

    // test pre-increment operator
    auto it2 = ++it1;
    CPPUNIT_ASSERT(*it1 == b);
    CPPUNIT_ASSERT(*it2 == b);

    // test post-increment operator
    auto it3 = it2++;
    CPPUNIT_ASSERT(*it2 == c);
    CPPUNIT_ASSERT(*it3 == b);

    // test begin()
    it3.begin();
    CPPUNIT_ASSERT(*it3 == a);
    CPPUNIT_ASSERT(it3 == list.begin());

    // test end()
    it3.end();
    CPPUNIT_ASSERT(it3 == list.end());

    // test loop with pre-increment operator
    {
        std::vector<libdnf5::rpm::Reldep> result;
        for (auto it = list.begin(), end = list.end(); it != end; ++it) {
            result.push_back(*it);
        }
        CPPUNIT_ASSERT(result == expected);
    }

    // test loop with post-increment operator
    {
        std::vector<libdnf5::rpm::Reldep> result;
        for (auto it = list.begin(), end = list.end(); it != end; it++) {
            result.push_back(*it);
        }
        CPPUNIT_ASSERT(result == expected);
    }
}

// add_reldep_with_glob uses libsolvs Dataiterator which needs the actual packages
void ReldepListTest::test_add_reldep_with_glob() {
    add_repo_solv("solv-repo1");

    libdnf5::rpm::ReldepList list(base);
    list.add_reldep_with_glob("pkg*");

    const std::vector<Reldep> expected = {
        Reldep(base, "pkg"),
        Reldep(base, "pkg-libs"),
        Reldep(base, "pkg.conf"),
        Reldep(base, "pkg.conf.d"),
        Reldep(base, "pkg-libs"),
    };
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(list));
}
