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


#include "test_advisory.hpp"

#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/package_set.hpp>

#include <filesystem>
#include <set>
#include <vector>


CPPUNIT_TEST_SUITE_REGISTRATION(AdvisoryAdvisoryTest);

//This allows running only this single test suite, by using `getRegistry("AdvisoryAdvisoryTest_suite")` in run_tests.cpp
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(AdvisoryAdvisoryTest, "AdvisoryAdvisoryTest_suite");

void AdvisoryAdvisoryTest::setUp() {
    BaseTestCase::setUp();
    BaseTestCase::add_repo_repomd("repomd-repo1");
}

void AdvisoryAdvisoryTest::test_get_name() {
    // Tests get_name method
    libdnf5::advisory::AdvisoryQuery advisories(base);
    advisories.filter_type("security");
    libdnf5::advisory::Advisory advisory = *advisories.begin();
    CPPUNIT_ASSERT_EQUAL(advisory.get_name(), std::string("DNF-2019-1"));
}

void AdvisoryAdvisoryTest::test_get_type() {
    // Tests get_type method
    libdnf5::advisory::AdvisoryQuery advisories(base);
    advisories.filter_type("security");
    libdnf5::advisory::Advisory advisory = *advisories.begin();
    CPPUNIT_ASSERT_EQUAL(advisory.get_type(), std::string("security"));
}

void AdvisoryAdvisoryTest::test_get_severity() {
    // Tests get_severity method
    libdnf5::advisory::AdvisoryQuery advisories(base);
    advisories.filter_type("security");
    libdnf5::advisory::Advisory advisory = *advisories.begin();
    CPPUNIT_ASSERT_EQUAL(advisory.get_severity(), std::string("moderate"));
}

void AdvisoryAdvisoryTest::test_get_references() {
    // Tests get_references method
    libdnf5::advisory::AdvisoryQuery advisories(base);
    advisories.filter_type("security");
    libdnf5::advisory::Advisory advisory = *advisories.begin();
    std::vector<libdnf5::advisory::AdvisoryReference> refs = advisory.get_references();
    CPPUNIT_ASSERT_EQUAL((size_t)1, refs.size());

    libdnf5::advisory::AdvisoryReference r = refs[0];
    CPPUNIT_ASSERT_EQUAL(std::string("1111"), r.get_id());
    CPPUNIT_ASSERT_EQUAL(std::string("cve"), r.get_type());
    CPPUNIT_ASSERT_EQUAL(std::string("CVE-2999"), r.get_title());
    CPPUNIT_ASSERT_EQUAL(std::string("https://foobar/foobarupdate_2"), r.get_url());
}

void AdvisoryAdvisoryTest::test_get_collections() {
    // Tests get_collections method
    libdnf5::advisory::AdvisoryQuery advisories(base);
    advisories.filter_type("security");
    libdnf5::advisory::Advisory advisory = *advisories.begin();
    std::vector<libdnf5::advisory::AdvisoryCollection> colls = advisory.get_collections();
    CPPUNIT_ASSERT_EQUAL((size_t)1, colls.size());

    libdnf5::advisory::AdvisoryCollection c = colls[0];
    std::vector<libdnf5::advisory::AdvisoryPackage> pkgs = c.get_packages();
    CPPUNIT_ASSERT_EQUAL((size_t)2, pkgs.size());
    CPPUNIT_ASSERT_EQUAL(std::string("pkg"), pkgs[0].get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("filesystem"), pkgs[1].get_name());

    std::vector<libdnf5::advisory::AdvisoryModule> mods = c.get_modules();
    CPPUNIT_ASSERT_EQUAL((size_t)2, mods.size());
    CPPUNIT_ASSERT_EQUAL(std::string("perl-DBI"), mods[0].get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("ethereum"), mods[1].get_name());

    libdnf5::advisory::AdvisoryQuery adv_query(base);
    adv_query.filter_name("DNF-2020-1");
    CPPUNIT_ASSERT_EQUAL((size_t)1, adv_query.size());
    colls = (*adv_query.begin()).get_collections();
    CPPUNIT_ASSERT_EQUAL((size_t)2, colls.size());

    libdnf5::advisory::AdvisoryCollection c1 = colls[0];
    pkgs.clear();
    pkgs = c1.get_packages();
    CPPUNIT_ASSERT_EQUAL((size_t)2, pkgs.size());
    CPPUNIT_ASSERT_EQUAL(std::string("wget"), pkgs[0].get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("yum"), pkgs[1].get_name());

    mods.clear();
    mods = c1.get_modules();
    CPPUNIT_ASSERT_EQUAL((size_t)1, mods.size());
    CPPUNIT_ASSERT_EQUAL(std::string("perl-DBI"), mods[0].get_name());

    libdnf5::advisory::AdvisoryCollection c2 = colls[1];
    pkgs.clear();
    pkgs = c2.get_packages();
    CPPUNIT_ASSERT_EQUAL((size_t)1, pkgs.size());
    CPPUNIT_ASSERT_EQUAL(std::string("bitcoin"), pkgs[0].get_name());

    mods.clear();
    mods = c2.get_modules();
    CPPUNIT_ASSERT_EQUAL((size_t)1, mods.size());
    CPPUNIT_ASSERT_EQUAL(std::string("perl"), mods[0].get_name());
}
