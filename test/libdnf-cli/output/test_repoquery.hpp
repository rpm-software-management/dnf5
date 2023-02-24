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


#ifndef TEST_LIBDNF_CLI_OUTPUT_REPOQUERY_HPP
#define TEST_LIBDNF_CLI_OUTPUT_REPOQUERY_HPP


#include "../../shared/base_test_case.hpp"

#include "libdnf/rpm/package_query.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>


class RepoqueryTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(RepoqueryTest);

    CPPUNIT_TEST(test_format_set_with_simple_str);
    CPPUNIT_TEST(test_format_set_with_tags);
    CPPUNIT_TEST(test_format_set_with_invalid_tags);
    CPPUNIT_TEST(test_format_set_with_tags_with_spacing);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_format_set_with_simple_str();
    void test_format_set_with_tags();
    void test_format_set_with_invalid_tags();
    void test_format_set_with_tags_with_spacing();

private:
    std::unique_ptr<libdnf::rpm::PackageQuery> pkgs;
};


#endif  // TEST_LIBDNF_CLI_OUTPUT_REPOQUERY_HPP
