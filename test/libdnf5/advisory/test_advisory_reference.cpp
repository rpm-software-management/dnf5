// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#include "test_advisory_reference.hpp"

#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/package_set.hpp>

#include <filesystem>
#include <set>
#include <vector>


CPPUNIT_TEST_SUITE_REGISTRATION(AdvisoryAdvisoryReferenceTest);

//This allows running only this single test suite, by using `getRegistry("AdvisoryAdvisoryReferenceTest_suite")` in run_tests.cpp
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(AdvisoryAdvisoryReferenceTest, "AdvisoryAdvisoryReferenceTest_suite");

void AdvisoryAdvisoryReferenceTest::setUp() {
    BaseTestCase::setUp();
    BaseTestCase::add_repo_repomd("repomd-repo1");
    libdnf5::advisory::AdvisoryQuery advisories(base);
    advisories.filter_name("DNF-2019-1");
    libdnf5::advisory::Advisory advisory = *advisories.begin();
    references = advisory.get_references();
}

void AdvisoryAdvisoryReferenceTest::test_get_id() {
    // Tests get_id method
    CPPUNIT_ASSERT_EQUAL(std::string("1111"), references[0].get_id());
}

void AdvisoryAdvisoryReferenceTest::test_get_type() {
    // Tests get_type method
    CPPUNIT_ASSERT_EQUAL(std::string("cve"), references[0].get_type());
}

void AdvisoryAdvisoryReferenceTest::test_get_title() {
    // Tests get_title method
    CPPUNIT_ASSERT_EQUAL(std::string("CVE-2999"), references[0].get_title());
}

void AdvisoryAdvisoryReferenceTest::test_get_url() {
    // Tests get_url method
    CPPUNIT_ASSERT_EQUAL(std::string("https://foobar/foobarupdate_2"), references[0].get_url());
}
