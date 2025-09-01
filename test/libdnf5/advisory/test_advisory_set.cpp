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


#include "test_advisory_set.hpp"

#include <libdnf5/advisory/advisory.hpp>

#include <filesystem>
#include <vector>


CPPUNIT_TEST_SUITE_REGISTRATION(AdvisoryAdvisorySetTest);

namespace {

// make constructor public so we can create Advisory instances in the tests
class TestAdvisory : public libdnf5::advisory::Advisory {
public:
    TestAdvisory(libdnf5::Base & base, libdnf5::advisory::AdvisoryId id)
        : libdnf5::advisory::Advisory(base.get_weak_ptr(), id) {}
};

}  // namespace


void AdvisoryAdvisorySetTest::setUp() {
    BaseTestCase::setUp();
    // Use solv-24pkgs repo to create a pool with 24 slots (packages).
    // Even though the repo doesn't have any advisories we can manually
    // create our own and by specifying their ids in the 0 - 24 range
    // we can reuse the slots as advisories.
    add_repo_solv("solv-24pkgs");

    // set1 contains advisories 0 - 15
    set1 = std::make_unique<libdnf5::advisory::AdvisorySet>(base);
    for (int i = 0; i < 16; i++) {
        TestAdvisory adv(base, libdnf5::advisory::AdvisoryId(i));
        set1->add(adv);
    }

    // set2 contains advisories 8, 24
    set2 = std::make_unique<libdnf5::advisory::AdvisorySet>(base);

    TestAdvisory adv8(base, libdnf5::advisory::AdvisoryId(8));
    set2->add(adv8);

    TestAdvisory adv24(base, libdnf5::advisory::AdvisoryId(24));
    set2->add(adv24);
}


void AdvisoryAdvisorySetTest::test_add() {
    // add an Advisory that does not exist in a AdvisorySet
    TestAdvisory adv(base, libdnf5::advisory::AdvisoryId(24));
    CPPUNIT_ASSERT(set1->contains(adv) == false);
    set1->add(adv);
    CPPUNIT_ASSERT(set1->contains(adv) == true);

    // add an Advisory that exists in a AdvisorySet
    set1->add(adv);
    CPPUNIT_ASSERT(set1->contains(adv) == true);
}


void AdvisoryAdvisorySetTest::test_contains() {
    // AdvisorySet contains a Advisory
    TestAdvisory adv0(base, libdnf5::advisory::AdvisoryId(0));
    CPPUNIT_ASSERT(set1->contains(adv0) == true);

    // AdvisorySet does not contain a Advisory
    TestAdvisory adv16(base, libdnf5::advisory::AdvisoryId(16));
    CPPUNIT_ASSERT(set1->contains(adv16) == false);

    // AdvisorySet does not contain a Advisory does is out of range of underlying bitmap
    TestAdvisory adv123(base, libdnf5::advisory::AdvisoryId(123));
    CPPUNIT_ASSERT(set1->contains(adv123) == false);
}


void AdvisoryAdvisorySetTest::test_remove() {
    // remove a Advisory that exists in a AdvisorySet
    TestAdvisory adv0(base, libdnf5::advisory::AdvisoryId(0));

    CPPUNIT_ASSERT(set1->contains(adv0) == true);
    set1->remove(adv0);
    CPPUNIT_ASSERT(set1->contains(adv0) == false);

    // remove a Advisory that does not exist in a AdvisorySet
    set1->remove(adv0);
    CPPUNIT_ASSERT(set1->contains(adv0) == false);
}


void AdvisoryAdvisorySetTest::test_union() {
    std::vector<libdnf5::advisory::Advisory> expected;
    std::vector<libdnf5::advisory::Advisory> result;

    // expected advisories: 0-15, 24
    for (int i = 0; i < 16; i++) {
        TestAdvisory adv(base, libdnf5::advisory::AdvisoryId(i));
        expected.push_back(adv);
    }
    TestAdvisory adv24(base, libdnf5::advisory::AdvisoryId(24));
    expected.push_back(adv24);

    *set1 |= *set2;
    for (auto it = set1->begin(); it != set1->end(); it++) {
        result.push_back((*it));
    }
    CPPUNIT_ASSERT(result == expected);
}


void AdvisoryAdvisorySetTest::test_intersection() {
    std::vector<libdnf5::advisory::Advisory> expected;
    std::vector<libdnf5::advisory::Advisory> result;

    // expected advisories: 8
    TestAdvisory adv8(base, libdnf5::advisory::AdvisoryId(8));
    expected.push_back(adv8);

    *set1 &= *set2;
    for (auto it = set1->begin(); it != set1->end(); it++) {
        result.push_back((*it));
    }
    CPPUNIT_ASSERT(result == expected);
}


void AdvisoryAdvisorySetTest::test_difference() {
    std::vector<libdnf5::advisory::Advisory> expected;
    std::vector<libdnf5::advisory::Advisory> result;

    // expected advisories: 0-7, 9-15
    for (int i = 0; i < 16; i++) {
        if (i == 8) {
            continue;
        }
        TestAdvisory adv(base, libdnf5::advisory::AdvisoryId(i));
        expected.push_back(adv);
    }

    *set1 -= *set2;
    for (auto it = set1->begin(); it != set1->end(); it++) {
        result.push_back((*it));
    }
    CPPUNIT_ASSERT(result == expected);
}


void AdvisoryAdvisorySetTest::test_iterator() {
    std::vector<libdnf5::advisory::Advisory> expected;

    for (int i = 0; i < 16; i++) {
        TestAdvisory adv(base, libdnf5::advisory::AdvisoryId(i));
        expected.push_back(adv);
    }

    // check if begin() points to the first advisory
    auto it1 = set1->begin();
    CPPUNIT_ASSERT_EQUAL((*it1).get_id().id, 0);

    // test pre-increment operator
    auto it2 = ++it1;
    CPPUNIT_ASSERT_EQUAL((*it1).get_id().id, 1);
    CPPUNIT_ASSERT_EQUAL((*it2).get_id().id, 1);

    // test post-increment operator
    auto it3 = it2++;
    CPPUNIT_ASSERT_EQUAL((*it2).get_id().id, 2);
    CPPUNIT_ASSERT_EQUAL((*it3).get_id().id, 1);

    // test copy assignment
    it1 = it2;
    CPPUNIT_ASSERT_EQUAL((*it1).get_id().id, 2);

    // test begin()
    it1.begin();
    CPPUNIT_ASSERT_EQUAL((*it1).get_id().id, 0);
    CPPUNIT_ASSERT(it1 == set1->begin());

    // test end()
    it1.end();
    CPPUNIT_ASSERT(it1 == set1->end());

    // test loop with pre-increment operator
    {
        std::vector<libdnf5::advisory::Advisory> result;
        for (auto it = set1->begin(), end = set1->end(); it != end; ++it) {
            result.push_back(*it);
        }
        CPPUNIT_ASSERT(result == expected);
    }

    // test loop with post-increment operator
    {
        std::vector<libdnf5::advisory::Advisory> result;
        for (auto it = set1->begin(), end = set1->end(); it != end; it++) {
            result.push_back(*it);
        }
        CPPUNIT_ASSERT(result == expected);
    }
}
