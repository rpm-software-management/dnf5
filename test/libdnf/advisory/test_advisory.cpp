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


#include "test_advisory.hpp"

#include "libdnf/rpm/package_query.hpp"
#include "libdnf/rpm/package_set.hpp"

#include <filesystem>
#include <set>
#include <vector>


CPPUNIT_TEST_SUITE_REGISTRATION(AdvisoryAdvisoryTest);

//This allows running only this single test suite, by using `getRegistry("AdvisoryAdvisoryTest_suite")` in run_tests.cpp
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(AdvisoryAdvisoryTest, "AdvisoryAdvisoryTest_suite");

void AdvisoryAdvisoryTest::setUp() {
    LibdnfTestCase::setUp();
    LibdnfTestCase::add_repo_repomd("repomd-repo1");
    libdnf::advisory::AdvisoryQuery q =
        libdnf::advisory::AdvisoryQuery(base).filter_type(libdnf::advisory::Advisory::Type::SECURITY);
    advisories = q.get_advisories();
    advisory = &(advisories[0]);
}

void AdvisoryAdvisoryTest::test_get_name() {
    // Tests get_name method
    CPPUNIT_ASSERT_EQUAL(advisory->get_name(), std::string("DNF-2019-1"));
}

void AdvisoryAdvisoryTest::test_get_type() {
    // Tests get_type method
    CPPUNIT_ASSERT_EQUAL(advisory->get_type(), libdnf::advisory::Advisory::Type::SECURITY);
}

void AdvisoryAdvisoryTest::test_get_type_cstring() {
    // Tests get_type method
    CPPUNIT_ASSERT(!strcmp(advisory->get_type_cstring(), "security"));
}

void AdvisoryAdvisoryTest::test_get_severity() {
    // Tests get_severity method
    CPPUNIT_ASSERT_EQUAL(advisory->get_severity(), std::string("moderate"));
}

void AdvisoryAdvisoryTest::test_get_references() {
    // Tests get_references method
    std::vector<libdnf::advisory::AdvisoryReference> refs = advisory->get_references();
    CPPUNIT_ASSERT_EQUAL(1lu, refs.size());

    libdnf::advisory::AdvisoryReference r = refs[0];
    CPPUNIT_ASSERT_EQUAL(std::string("1111"), r.get_id());
    CPPUNIT_ASSERT_EQUAL(libdnf::advisory::AdvisoryReference::Type::CVE, r.get_type());
    CPPUNIT_ASSERT_EQUAL(std::string("CVE-2999"), r.get_title());
    CPPUNIT_ASSERT_EQUAL(std::string("https://foobar/foobarupdate_2"), r.get_url());
}

void AdvisoryAdvisoryTest::test_get_collections() {
    // Tests get_collections method
    std::vector<libdnf::advisory::AdvisoryCollection> colls = advisory->get_collections();
    CPPUNIT_ASSERT_EQUAL(1lu, colls.size());

    libdnf::advisory::AdvisoryCollection c = colls[0];
    std::vector<libdnf::advisory::AdvisoryPackage> pkgs = c.get_packages();
    CPPUNIT_ASSERT_EQUAL(2lu, pkgs.size());
    CPPUNIT_ASSERT_EQUAL(std::string("pkg"), pkgs[0].get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("filesystem"), pkgs[1].get_name());

    std::vector<libdnf::advisory::AdvisoryModule> mods = c.get_modules();
    CPPUNIT_ASSERT_EQUAL(2lu, mods.size());
    CPPUNIT_ASSERT_EQUAL(std::string("perl-DBI"), mods[0].get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("ethereum"), mods[1].get_name());

    auto adv_vec = libdnf::advisory::AdvisoryQuery(base).filter_name("DNF-2020-1").get_advisories();
    CPPUNIT_ASSERT_EQUAL(1lu, adv_vec.size());
    libdnf::advisory::Advisory * advisory2 = &(adv_vec[0]);
    colls = advisory2->get_collections();
    CPPUNIT_ASSERT_EQUAL(2lu, colls.size());

    libdnf::advisory::AdvisoryCollection c1 = colls[0];
    pkgs.clear();
    pkgs = c1.get_packages();
    CPPUNIT_ASSERT_EQUAL(2lu, pkgs.size());
    CPPUNIT_ASSERT_EQUAL(std::string("wget"), pkgs[0].get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("yum"), pkgs[1].get_name());

    mods.clear();
    mods = c1.get_modules();
    CPPUNIT_ASSERT_EQUAL(1lu, mods.size());
    CPPUNIT_ASSERT_EQUAL(std::string("perl-DBI"), mods[0].get_name());

    libdnf::advisory::AdvisoryCollection c2 = colls[1];
    pkgs.clear();
    pkgs = c2.get_packages();
    CPPUNIT_ASSERT_EQUAL(1lu, pkgs.size());
    CPPUNIT_ASSERT_EQUAL(std::string("bitcoin"), pkgs[0].get_name());

    mods.clear();
    mods = c2.get_modules();
    CPPUNIT_ASSERT_EQUAL(1lu, mods.size());
    CPPUNIT_ASSERT_EQUAL(std::string("perl"), mods[0].get_name());
}
