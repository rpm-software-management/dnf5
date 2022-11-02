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


#include "test_advisory_query.hpp"

#include "utils.hpp"

#include "libdnf/rpm/package_query.hpp"
#include "libdnf/rpm/package_set.hpp"

#include <filesystem>
#include <set>
#include <vector>


CPPUNIT_TEST_SUITE_REGISTRATION(AdvisoryAdvisoryQueryTest);

using namespace libdnf::advisory;

void AdvisoryAdvisoryQueryTest::setUp() {
    BaseTestCase::setUp();
    BaseTestCase::add_repo_repomd("repomd-repo1");
}

void AdvisoryAdvisoryQueryTest::test_size() {
    auto adv_query = AdvisoryQuery(base);
    std::vector<Advisory> expected = {
        get_advisory("DNF-2019-1"), get_advisory("DNF-2020-1"), get_advisory("PKG-NEWER"), get_advisory("PKG-OLDER")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));
}

void AdvisoryAdvisoryQueryTest::test_filter_name() {
    // Tests filter_name method
    AdvisoryQuery adv_query(base);
    adv_query.filter_name("*2020-1", libdnf::sack::QueryCmp::GLOB);
    std::vector<Advisory> expected = {get_advisory("DNF-2020-1")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));

    adv_query = AdvisoryQuery(base);
    adv_query.filter_name("DNF-20*", libdnf::sack::QueryCmp::GLOB);
    expected = {get_advisory("DNF-2019-1"), get_advisory("DNF-2020-1")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));

    adv_query = AdvisoryQuery(base);
    adv_query.filter_name(std::vector<std::string>{"DNF-2019-1", "DNF-2020-1"}, libdnf::sack::QueryCmp::EQ);
    expected = {get_advisory("DNF-2019-1"), get_advisory("DNF-2020-1")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));
}

void AdvisoryAdvisoryQueryTest::test_filter_type() {
    // Tests filter_type method
    AdvisoryQuery adv_query(base);
    adv_query.filter_type("bugfix");
    std::vector<Advisory> expected = {get_advisory("DNF-2020-1"), get_advisory("PKG-OLDER")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));

    adv_query = AdvisoryQuery(base);
    adv_query.filter_type("enhancement");
    expected = {get_advisory("PKG-NEWER")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));

    adv_query = AdvisoryQuery(base);
    adv_query.filter_type(std::vector<std::string>{"bugfix", "security"}, libdnf::sack::QueryCmp::EQ);
    expected = {get_advisory("DNF-2019-1"), get_advisory("DNF-2020-1"), get_advisory("PKG-OLDER")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));
}

void AdvisoryAdvisoryQueryTest::test_filter_packages() {
    // Tests filter_packages method
    libdnf::rpm::PackageQuery pkg_query(base);

    AdvisoryQuery adv_query = AdvisoryQuery(base);
    adv_query.filter_packages(pkg_query, libdnf::sack::QueryCmp::GT);
    std::vector<Advisory> expected = {get_advisory("PKG-NEWER")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));

    adv_query = AdvisoryQuery(base);
    adv_query.filter_packages(pkg_query, libdnf::sack::QueryCmp::GTE);
    expected = {get_advisory("DNF-2019-1"), get_advisory("PKG-NEWER")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));

    adv_query = AdvisoryQuery(base);
    adv_query.filter_packages(pkg_query, libdnf::sack::QueryCmp::EQ);
    expected = {get_advisory("DNF-2019-1")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));

    adv_query = AdvisoryQuery(base);
    adv_query.filter_packages(pkg_query, libdnf::sack::QueryCmp::LTE);
    expected = {get_advisory("DNF-2019-1"), get_advisory("PKG-OLDER")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));

    adv_query = AdvisoryQuery(base);
    adv_query.filter_packages(pkg_query, libdnf::sack::QueryCmp::LT);
    expected = {get_advisory("PKG-OLDER")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));
}

void AdvisoryAdvisoryQueryTest::test_filter_cve() {
    // Tests filter_reference method with cve
    AdvisoryQuery adv_query(base);
    adv_query.filter_reference("3333", libdnf::sack::QueryCmp::EQ, "cve");
    std::vector<Advisory> expected = {get_advisory("DNF-2020-1")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));

    adv_query = AdvisoryQuery(base);
    adv_query.filter_reference(std::vector<std::string>{"1111", "3333"}, libdnf::sack::QueryCmp::EQ, "cve");
    expected = {get_advisory("DNF-2019-1"), get_advisory("DNF-2020-1")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));

    adv_query = AdvisoryQuery(base);
    adv_query.filter_reference(std::vector<std::string>{"1111", "4444"}, libdnf::sack::QueryCmp::EQ, "cve");
    expected = {get_advisory("DNF-2019-1")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));

    adv_query = AdvisoryQuery(base);
    adv_query.filter_reference("*", libdnf::sack::QueryCmp::GLOB, "cve");
    expected = {get_advisory("DNF-2019-1"), get_advisory("DNF-2020-1")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));
}

void AdvisoryAdvisoryQueryTest::test_filter_bugzilla() {
    // Tests filter_reference method with bugzilla
    AdvisoryQuery adv_query(base);
    adv_query.filter_reference("2222", libdnf::sack::QueryCmp::EQ, "bugzilla");
    std::vector<Advisory> expected = {get_advisory("DNF-2020-1")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));

    adv_query = AdvisoryQuery(base);
    adv_query.filter_reference(std::vector<std::string>{"1111", "3333"}, libdnf::sack::QueryCmp::EQ, "bugzilla");
    expected = {};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));

    adv_query = AdvisoryQuery(base);
    adv_query.filter_reference("*", libdnf::sack::QueryCmp::GLOB, "bugzilla");
    expected = {get_advisory("DNF-2020-1")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));
}

void AdvisoryAdvisoryQueryTest::test_filter_reference() {
    // Tests filter_reference method without type specified
    AdvisoryQuery adv_query(base);
    adv_query.filter_reference("2222");
    std::vector<Advisory> expected = {get_advisory("DNF-2020-1")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));

    adv_query = AdvisoryQuery(base);
    adv_query.filter_reference(std::vector<std::string>{"1111", "3333"});
    expected = {get_advisory("DNF-2019-1"), get_advisory("DNF-2020-1")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));

    adv_query = AdvisoryQuery(base);
    adv_query.filter_reference("*", libdnf::sack::QueryCmp::GLOB);
    expected = {get_advisory("DNF-2019-1"), get_advisory("DNF-2020-1")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));

    adv_query = AdvisoryQuery(base);
    adv_query.filter_reference("none*", libdnf::sack::QueryCmp::GLOB);
    expected = {};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));
}

void AdvisoryAdvisoryQueryTest::test_filter_severity() {
    // Tests filter_severity method
    AdvisoryQuery adv_query(base);
    adv_query.filter_severity("moderate", libdnf::sack::QueryCmp::EQ);
    std::vector<Advisory> expected = {get_advisory("DNF-2019-1")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));

    adv_query = AdvisoryQuery(base);
    adv_query.filter_severity(std::vector<std::string>{"moderate", "critical"}, libdnf::sack::QueryCmp::EQ);
    expected = {get_advisory("DNF-2019-1"), get_advisory("DNF-2020-1")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(adv_query));
}

void AdvisoryAdvisoryQueryTest::test_get_advisory_packages_sorted() {
    // Tests get_advisory_packages_sorted method
    libdnf::rpm::PackageQuery pkg_query(base);
    // pkg_query contains: pkg-1.2-3.x86_64, pkg-libs-1:1.3-4.x86_64, unresolvable-1:2-3.noarch
    std::vector<AdvisoryPackage> adv_pkgs =
        AdvisoryQuery(base).get_advisory_packages_sorted(pkg_query, libdnf::sack::QueryCmp::GTE);
    CPPUNIT_ASSERT_EQUAL((size_t)2, adv_pkgs.size());
    CPPUNIT_ASSERT_EQUAL(std::string("pkg"), adv_pkgs[0].get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("1.2-3"), adv_pkgs[0].get_evr());
    CPPUNIT_ASSERT_EQUAL(std::string("pkg"), adv_pkgs[1].get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("4.0-1"), adv_pkgs[1].get_evr());

    adv_pkgs = AdvisoryQuery(base).get_advisory_packages_sorted(pkg_query, libdnf::sack::QueryCmp::LTE);
    CPPUNIT_ASSERT_EQUAL((size_t)2, adv_pkgs.size());
    CPPUNIT_ASSERT_EQUAL(std::string("pkg"), adv_pkgs[0].get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("1.2-3"), adv_pkgs[0].get_evr());
    CPPUNIT_ASSERT_EQUAL(std::string("pkg"), adv_pkgs[1].get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("0.1-1"), adv_pkgs[1].get_evr());
}
