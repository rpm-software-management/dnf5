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


#include "test_group_query.hpp"

#include "../shared/utils.hpp"

#include <libdnf5/comps/group/query.hpp>


CPPUNIT_TEST_SUITE_REGISTRATION(CompsGroupQueryTest);


using namespace libdnf5::comps;


void CompsGroupQueryTest::setUp() {
    BaseTestCase::setUp();
    add_repo_repomd("repomd-comps-core", false);
    add_repo_repomd("repomd-comps-core-empty", false);
    add_repo_repomd("repomd-comps-core-different-translations", false);
    add_repo_repomd("repomd-comps-critical-path-standard", false);
    add_repo_repomd("repomd-comps-standard");
}


void CompsGroupQueryTest::test_query_all() {
    GroupQuery q_groups(base);
    std::vector<Group> expected = {get_group("core"), get_group("critical-path-standard"), get_group("standard")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));
}


void CompsGroupQueryTest::test_query_filter_groupid() {
    // Filter groups with id equal to "standard"
    GroupQuery q_groups(base);
    q_groups.filter_groupid("standard");
    std::vector<Group> expected = {get_group("standard")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));

    // Filter groups with id containing "standard"
    q_groups = GroupQuery(base);
    q_groups.filter_groupid("standard", libdnf5::sack::QueryCmp::CONTAINS);
    expected = {get_group("critical-path-standard"), get_group("standard")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));

    // Filter groups with id matching glob "*standard"
    q_groups = GroupQuery(base);
    q_groups.filter_groupid("*standard", libdnf5::sack::QueryCmp::GLOB);
    expected = {get_group("critical-path-standard"), get_group("standard")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));

    // Filter groups with id matching glob "standard*"
    q_groups = GroupQuery(base);
    q_groups.filter_groupid("standard*", libdnf5::sack::QueryCmp::GLOB);
    expected = {get_group("standard")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));

    // Filter groups with id equal to "standard" or "core"
    q_groups = GroupQuery(base);
    q_groups.filter_groupid(std::vector<std::string>({"standard", "core"}));
    expected = {get_group("core"), get_group("standard")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));
}


void CompsGroupQueryTest::test_query_filter_name() {
    // Filter groups with name equal to "Standard"
    GroupQuery q_groups(base);
    q_groups.filter_name("Standard");
    std::vector<Group> expected = {get_group("standard")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));

    // Filter groups with name containing "Standard"
    q_groups = GroupQuery(base);
    q_groups.filter_name("Standard", libdnf5::sack::QueryCmp::CONTAINS);
    expected = {get_group("critical-path-standard"), get_group("standard")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));

    // Filter groups with name matching glob "*Standard*"
    q_groups = GroupQuery(base);
    q_groups.filter_name("*Standard*", libdnf5::sack::QueryCmp::GLOB);
    expected = {get_group("critical-path-standard"), get_group("standard")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));

    // Filter groups with name equal to "Standard" or "Core"
    q_groups = GroupQuery(base);
    q_groups.filter_name(std::vector<std::string>({"Standard", "Core"}));
    expected = {get_group("core"), get_group("standard")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));
}


void CompsGroupQueryTest::test_query_filter_uservisible() {
    // Filter groups with uservisible=true
    GroupQuery q_groups(base);
    q_groups.filter_uservisible(true);
    std::vector<Group> expected = {get_group("core"), get_group("critical-path-standard")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));

    // Filter groups with uservisible=false
    q_groups = GroupQuery(base);
    q_groups.filter_uservisible(false);
    expected = {get_group("standard")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));
}


void CompsGroupQueryTest::test_query_filter_default() {
    // Filter groups with default=true
    GroupQuery q_groups(base);
    q_groups.filter_default(true);
    std::vector<Group> expected = {get_group("critical-path-standard")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));

    // Filter groups with default=false
    q_groups = GroupQuery(base);
    q_groups.filter_default(false);
    expected = {get_group("core"), get_group("standard")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));
}


void CompsGroupQueryTest::test_query_filter_package_name() {
    // Filter groups which contain given packages
    GroupQuery q_groups(base);
    q_groups.filter_package_name(std::vector<std::string>({"chrony"}));
    std::vector<Group> expected = {get_group("standard")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));

    q_groups = GroupQuery(base);
    q_groups.filter_package_name(std::vector<std::string>({"chrony", "fprintd*"}), libdnf5::sack::QueryCmp::IGLOB);
    expected = {get_group("critical-path-standard"), get_group("standard")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));
}
