// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_CLI_OUTPUT_REPOQUERY_HPP
#define TEST_LIBDNF5_CLI_OUTPUT_REPOQUERY_HPP


#include "../../shared/base_test_case.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/rpm/package_query.hpp>

#include <string>


class RepoqueryTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(RepoqueryTest);

    CPPUNIT_TEST(test_format_set_with_simple_str);
    CPPUNIT_TEST(test_format_set_with_tags);
    CPPUNIT_TEST(test_format_set_with_invalid_tags);
    CPPUNIT_TEST(test_format_set_with_tags_with_spacing);
    CPPUNIT_TEST(test_pkg_attr_uniq_sorted);
    CPPUNIT_TEST(test_requires_filelists);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_format_set_with_simple_str();
    void test_format_set_with_tags();
    void test_format_set_with_invalid_tags();
    void test_format_set_with_tags_with_spacing();
    void test_pkg_attr_uniq_sorted();
    void test_requires_filelists();

private:
    std::unique_ptr<libdnf5::rpm::PackageQuery> pkgs;
};


#endif  // TEST_LIBDNF5_CLI_OUTPUT_REPOQUERY_HPP
