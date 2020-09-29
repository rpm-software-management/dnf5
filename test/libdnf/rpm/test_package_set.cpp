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


#include "test_package_set.hpp"

#include "libdnf/rpm/package.hpp"

#include <filesystem>
#include <vector>


CPPUNIT_TEST_SUITE_REGISTRATION(RpmPackageSetTest);

// make constructor public so we can create Package instances in the tests
class TestPackage : public libdnf::rpm::Package {
public:
    TestPackage(libdnf::rpm::SolvSack * sack, libdnf::rpm::PackageId id) : libdnf::rpm::Package(sack, id) {}
};


void RpmPackageSetTest::setUp() {
    RepoFixture::setUp();
    add_repo("dnf-ci-fedora");

    // set1 contains packages 0 - 15
    set1 = std::make_unique<libdnf::rpm::PackageSet>(sack);
    for (int i = 0; i < 16; i++) {
        TestPackage pkg(sack, libdnf::rpm::PackageId(i));
        set1->add(pkg);
    }

    // set2 contains packages 8, 24
    set2 = std::make_unique<libdnf::rpm::PackageSet>(sack);

    TestPackage pkg8(sack, libdnf::rpm::PackageId(8));
    set2->add(pkg8);

    TestPackage pkg24(sack, libdnf::rpm::PackageId(24));
    set2->add(pkg24);
}


void RpmPackageSetTest::test_add() {
    // add a Package that does not exist in a PackageSet
    TestPackage pkg(sack, libdnf::rpm::PackageId(24));
    CPPUNIT_ASSERT(set1->contains(pkg) == false);
    set1->add(pkg);
    CPPUNIT_ASSERT(set1->contains(pkg) == true);

    // add a Package that exists in a PackageSet
    set1->add(pkg);
    CPPUNIT_ASSERT(set1->contains(pkg) == true);
}


void RpmPackageSetTest::test_contains() {
    // PackageSet contains a Package
    TestPackage pkg0(sack, libdnf::rpm::PackageId(0));
    CPPUNIT_ASSERT(set1->contains(pkg0) == true);

    // PackageSet does not contain a Package
    TestPackage pkg16(sack, libdnf::rpm::PackageId(16));
    CPPUNIT_ASSERT(set1->contains(pkg16) == false);

    // PackageSet does not contain a Package does is out of range of underlying bitmap
    TestPackage pkg123(sack, libdnf::rpm::PackageId(123));
    CPPUNIT_ASSERT(set1->contains(pkg123) == false);
}


void RpmPackageSetTest::test_remove() {
    // remove a Package that exists in a PackageSet
    TestPackage pkg0(sack, libdnf::rpm::PackageId(0));

    CPPUNIT_ASSERT(set1->contains(pkg0) == true);
    set1->remove(pkg0);
    CPPUNIT_ASSERT(set1->contains(pkg0) == false);

    // remove a Package that does not exist in a PackageSet
    set1->remove(pkg0);
    CPPUNIT_ASSERT(set1->contains(pkg0) == false);
}


void RpmPackageSetTest::test_union() {
    std::vector<libdnf::rpm::Package> expected;
    std::vector<libdnf::rpm::Package> result;

    // expected packages: 0-15, 24
    for (int i = 0; i < 16; i++) {
        TestPackage pkg(sack, libdnf::rpm::PackageId(i));
        expected.push_back(pkg);
    }
    TestPackage pkg24(sack, libdnf::rpm::PackageId(24));
    expected.push_back(pkg24);

    *set1 |= *set2;
    for (auto it = set1->begin(); it != set1->end(); it++) {
        result.push_back((*it));
    }
    CPPUNIT_ASSERT(result == expected);
}


void RpmPackageSetTest::test_intersection() {
    std::vector<libdnf::rpm::Package> expected;
    std::vector<libdnf::rpm::Package> result;

    // expected packages: 8
    TestPackage pkg8(sack, libdnf::rpm::PackageId(8));
    expected.push_back(pkg8);

    *set1 &= *set2;
    for (auto it = set1->begin(); it != set1->end(); it++) {
        result.push_back((*it));
    }
    CPPUNIT_ASSERT(result == expected);
}


void RpmPackageSetTest::test_difference() {
    std::vector<libdnf::rpm::Package> expected;
    std::vector<libdnf::rpm::Package> result;

    // expected packages: 0-7, 9-15
    for (int i = 0; i < 16; i++) {
        if (i == 8) {
            continue;
        }
        TestPackage pkg(sack, libdnf::rpm::PackageId(i));
        expected.push_back(pkg);
    }

    *set1 -= *set2;
    for (auto it = set1->begin(); it != set1->end(); it++) {
        result.push_back((*it));
    }
    CPPUNIT_ASSERT(result == expected);
}


void RpmPackageSetTest::test_iterator() {
    std::vector<libdnf::rpm::Package> expected;
    std::vector<libdnf::rpm::Package> result;

    for (int i = 0; i < 16; i++) {
        TestPackage pkg(sack, libdnf::rpm::PackageId(i));
        expected.push_back(pkg);
    }

    // check if begin() points to the first package
    auto beg = set1->begin();
    CPPUNIT_ASSERT((*beg).get_id().id == 0);

    // test if end() points to package with id == -2
    auto end = set1->end();
    CPPUNIT_ASSERT((*end).get_id().id == -2);

    // test if increasing an iterator moves to the second package
    ++beg;
    CPPUNIT_ASSERT((*beg).get_id().id == 1);

    // test iterating the whole package set
    for (auto it = set1->begin(); it != set1->end(); it++) {
        result.push_back((*it));
    }
    CPPUNIT_ASSERT(result == expected);
}
