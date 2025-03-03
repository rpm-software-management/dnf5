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


#include "test_environment_query.hpp"

#include "../shared/utils.hpp"

#include <libdnf5/comps/environment/query.hpp>


CPPUNIT_TEST_SUITE_REGISTRATION(CompsEnvironmentQueryTest);


using namespace libdnf5::comps;


void CompsEnvironmentQueryTest::setUp() {
    BaseTestCase::setUp();
    add_repo_repomd("repomd-comps-core-environment", false);
    add_repo_repomd("repomd-comps-custom-environment", false);
    add_repo_repomd("repomd-comps-minimal-environment", false);
    add_repo_repomd("repomd-comps-minimal-environment-empty", false);
    add_repo_repomd("repomd-comps-minimal-environment-v2");
}


void CompsEnvironmentQueryTest::test_query_all() {
    EnvironmentQuery q_environments(base);
    std::vector<Environment> expected = {
        get_environment("core"), get_environment("custom-environment"), get_environment("minimal-environment")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_environments));
}


void CompsEnvironmentQueryTest::test_query_filter_environmentid() {
    // Filter envitonments with id equal to "core"
    EnvironmentQuery q_environments(base);
    q_environments.filter_environmentid("core");
    std::vector<Environment> expected = {get_environment("core")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_environments));

    // Filter envitonments with id containing "environment"
    q_environments = EnvironmentQuery(base);
    q_environments.filter_environmentid("environment", libdnf5::sack::QueryCmp::CONTAINS);
    expected = {get_environment("custom-environment"), get_environment("minimal-environment")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_environments));

    // Filter envitonments with id matching glob "*environment"
    q_environments = EnvironmentQuery(base);
    q_environments.filter_environmentid("*environment", libdnf5::sack::QueryCmp::GLOB);
    expected = {get_environment("custom-environment"), get_environment("minimal-environment")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_environments));

    // Filter envitonments with id equal to "custom-environment" or "core"
    q_environments = EnvironmentQuery(base);
    q_environments.filter_environmentid(std::vector<std::string>{"custom-environment", "core"});
    expected = {get_environment("core"), get_environment("custom-environment")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_environments));
}


void CompsEnvironmentQueryTest::test_query_filter_name() {
    // Filter envitonments with name equal to "Core Environment"
    EnvironmentQuery q_environments(base);
    q_environments.filter_name("Core Environment");
    std::vector<Environment> expected = {get_environment("core")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_environments));

    // Filter envitonments with name containing "Environment"
    q_environments = EnvironmentQuery(base);
    q_environments.filter_name("Custom", libdnf5::sack::QueryCmp::CONTAINS);
    expected = {get_environment("custom-environment")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_environments));

    // Filter envitonments with name matching glob "*Environment"
    q_environments = EnvironmentQuery(base);
    q_environments.filter_name("Custom*", libdnf5::sack::QueryCmp::GLOB);
    expected = {get_environment("custom-environment")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_environments));

    // Filter envitonments with name equal to "Custom Environment" or "Core Environment"
    q_environments = EnvironmentQuery(base);
    q_environments.filter_name(std::vector<std::string>{"Custom Operating System", "Core Environment"});
    expected = {get_environment("core"), get_environment("custom-environment")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_environments));
}


void CompsEnvironmentQueryTest::test_query_excludes() {
    auto sack = base.get_comps_sack();

    // Set user excludes to environment "standard" -> user excludes are groups: "standard"
    EnvironmentQuery q_excludes1(base);
    q_excludes1.filter_environmentid("custom-environment");
    sack->set_user_environment_excludes(q_excludes1);
    EnvironmentQuery q_groups(base);
    std::vector<Environment> expected = {get_environment("core"), get_environment("minimal-environment")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));

    // Add environment "core" to user excludes -> user excludes are groups: "standard", "core"
    EnvironmentQuery q_excludes2(base);
    q_excludes2.filter_environmentid("core");
    sack->add_user_environment_excludes(q_excludes2);
    q_groups = EnvironmentQuery(base);
    expected = {get_environment("minimal-environment")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));

    // Remove environment "custom-environment" from user excludes -> user excludes are groups: "core"
    sack->remove_user_environment_excludes(q_excludes1);
    q_groups = EnvironmentQuery(base);
    expected = {get_environment("custom-environment"), get_environment("minimal-environment")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));

    // Set user excludes to environment "custom-environment" -> user excludes are groups: "custom-environment"
    sack->set_user_environment_excludes(q_excludes1);
    q_groups = EnvironmentQuery(base);
    expected = {get_environment("core"), get_environment("minimal-environment")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));

    // Clear user excludes -> user excludes are empty
    sack->clear_user_environment_excludes();
    q_groups = EnvironmentQuery(base);
    expected = {get_environment("core"), get_environment("custom-environment"), get_environment("minimal-environment")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(q_groups));
}
