/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "test_environment.hpp"

#include <libdnf5/comps/environment/query.hpp>

#include <filesystem>
#include <fstream>


CPPUNIT_TEST_SUITE_REGISTRATION(CompsEnvironmentTest);


using namespace libdnf5::comps;


void CompsEnvironmentTest::test_load() {
    add_repo_repomd("repomd-comps-minimal-environment");

    EnvironmentQuery q_minimal_env(base);
    q_minimal_env.filter_installed(false);
    q_minimal_env.filter_environmentid("minimal-environment");
    auto minimal_env = q_minimal_env.get();
    CPPUNIT_ASSERT_EQUAL(std::string("minimal-environment"), minimal_env.get_environmentid());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimal Install"), minimal_env.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimale Installation"), minimal_env.get_translated_name("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("Basic functionality."), minimal_env.get_description());
    CPPUNIT_ASSERT_EQUAL(std::string("Grundlegende Funktionalität."), minimal_env.get_translated_description("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("3"), minimal_env.get_order());
    CPPUNIT_ASSERT_EQUAL(false, minimal_env.get_installed());
}


void CompsEnvironmentTest::test_load_defaults() {
    add_repo_repomd("repomd-comps-minimal-environment-empty");

    EnvironmentQuery q_minimal_empty(base);
    q_minimal_empty.filter_environmentid("minimal-environment");
    auto minimal_empty = q_minimal_empty.get();
    CPPUNIT_ASSERT_EQUAL(std::string("minimal-environment"), minimal_empty.get_environmentid());
    CPPUNIT_ASSERT_EQUAL(std::string(""), minimal_empty.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string(""), minimal_empty.get_translated_name("ja"));
    CPPUNIT_ASSERT_EQUAL(std::string(""), minimal_empty.get_description());
    CPPUNIT_ASSERT_EQUAL(std::string(""), minimal_empty.get_translated_description("ja"));
    CPPUNIT_ASSERT_EQUAL(std::string(""), minimal_empty.get_order());
    CPPUNIT_ASSERT_EQUAL(false, minimal_empty.get_installed());
}

void CompsEnvironmentTest::test_merge() {
    // Load multiple different definitions of the minimal environment
    add_repo_repomd("repomd-comps-minimal-environment", false);
    add_repo_repomd("repomd-comps-custom-environment", false);
    add_repo_repomd("repomd-comps-minimal-environment-v2");

    // The "Minimal Install v2" is preferred because its repoid is alphabetically higher
    EnvironmentQuery q_minimal_env(base);
    q_minimal_env.filter_environmentid("minimal-environment");
    auto minimal_env = q_minimal_env.get();
    CPPUNIT_ASSERT_EQUAL(std::string("minimal-environment"), minimal_env.get_environmentid());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimal Install v2"), minimal_env.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimale Installation v2"), minimal_env.get_translated_name("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("Basic functionality v2."), minimal_env.get_description());
    CPPUNIT_ASSERT_EQUAL(std::string("Grundlegende Funktionalität v2."), minimal_env.get_translated_description("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("4"), minimal_env.get_order());
    CPPUNIT_ASSERT_EQUAL(false, minimal_env.get_installed());
}

void CompsEnvironmentTest::test_merge_when_different_load_order() {
    // Load multiple different definitions of the minimal environment
    // The order of loading the repositories does not matter
    add_repo_repomd("repomd-comps-minimal-environment-v2", false);
    add_repo_repomd("repomd-comps-custom-environment", false);
    add_repo_repomd("repomd-comps-minimal-environment");

    // The "Minimal Install v2" is preferred because its repoid is alphabetically higher
    EnvironmentQuery q_minimal_env(base);
    q_minimal_env.filter_environmentid("minimal-environment");
    auto minimal_env = q_minimal_env.get();
    CPPUNIT_ASSERT_EQUAL(std::string("minimal-environment"), minimal_env.get_environmentid());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimal Install v2"), minimal_env.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimale Installation v2"), minimal_env.get_translated_name("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("Basic functionality v2."), minimal_env.get_description());
    CPPUNIT_ASSERT_EQUAL(std::string("Grundlegende Funktionalität v2."), minimal_env.get_translated_description("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("4"), minimal_env.get_order());
    CPPUNIT_ASSERT_EQUAL(false, minimal_env.get_installed());
}


void CompsEnvironmentTest::test_merge_with_empty() {
    // Load minimal environment and another definition with all attributes empty
    add_repo_repomd("repomd-comps-minimal-environment", false);
    add_repo_repomd("repomd-comps-custom-environment", false);
    add_repo_repomd("repomd-comps-minimal-environment-empty");

    // All the attributes are taken from the non-empty definition
    EnvironmentQuery q_minimal_empty(base);
    q_minimal_empty.filter_environmentid("minimal-environment");
    auto minimal_empty = q_minimal_empty.get();
    CPPUNIT_ASSERT_EQUAL(std::string("minimal-environment"), minimal_empty.get_environmentid());
    // attributes are missing in minimal-environment-empty.xml -> original values are kept
    CPPUNIT_ASSERT_EQUAL(std::string("Minimal Install"), minimal_empty.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimale Installation"), minimal_empty.get_translated_name("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("Basic functionality."), minimal_empty.get_description());
    CPPUNIT_ASSERT_EQUAL(std::string("Grundlegende Funktionalität."), minimal_empty.get_translated_description("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("3"), minimal_empty.get_order());
    CPPUNIT_ASSERT_EQUAL(false, minimal_empty.get_installed());
}


void CompsEnvironmentTest::test_merge_empty_with_nonempty() {
    // Load minimal environment and another definition with all attributes empty
    add_repo_repomd("repomd-comps-minimal-environment-empty", false);
    add_repo_repomd("repomd-comps-custom-environment", false);
    add_repo_repomd("repomd-comps-minimal-environment");

    // All the attributes are taken from the non-empty definition
    EnvironmentQuery q_minimal_env(base);
    q_minimal_env.filter_installed(false);
    q_minimal_env.filter_environmentid("minimal-environment");
    auto minimal_env = q_minimal_env.get();
    CPPUNIT_ASSERT_EQUAL(std::string("minimal-environment"), minimal_env.get_environmentid());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimal Install"), minimal_env.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimale Installation"), minimal_env.get_translated_name("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("Basic functionality."), minimal_env.get_description());
    CPPUNIT_ASSERT_EQUAL(std::string("Grundlegende Funktionalität."), minimal_env.get_translated_description("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("3"), minimal_env.get_order());
    CPPUNIT_ASSERT_EQUAL(false, minimal_env.get_installed());
}


void CompsEnvironmentTest::test_merge_different_translations() {
    // Load different definitions of the minimal environment with different set of translations
    add_repo_repomd("repomd-comps-minimal-environment", false);
    add_repo_repomd("repomd-comps-minimal-environment-different-translations");

    EnvironmentQuery q_minimal_env(base);
    q_minimal_env.filter_environmentid("minimal-environment");
    auto minimal_env = q_minimal_env.get();
    CPPUNIT_ASSERT_EQUAL(std::string("minimal-environment"), minimal_env.get_environmentid());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimal Install"), minimal_env.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("Basic functionality."), minimal_env.get_description());
    // translations that are missing in minimal-environment-different-translations are taken from minimal-environment
    CPPUNIT_ASSERT_EQUAL(std::string("Minimale Installation"), minimal_env.get_translated_name("de"));
    CPPUNIT_ASSERT_EQUAL(std::string("Grundlegende Funktionalität."), minimal_env.get_translated_description("de"));
    // translations that are missing in minimal-environment are taken from minimal-environment-different-translations
    CPPUNIT_ASSERT_EQUAL(std::string("Vähimmäisasennus"), minimal_env.get_translated_name("fi"));
    CPPUNIT_ASSERT_EQUAL(std::string("Perustoiminnot."), minimal_env.get_translated_description("fi"));
}


void CompsEnvironmentTest::test_serialize() {
    add_repo_repomd("repomd-comps-minimal-environment");

    EnvironmentQuery q_minimal_env(base);
    q_minimal_env.filter_environmentid("minimal-environment");
    auto minimal_env = q_minimal_env.get();

    auto serialize_path = std::filesystem::temp_directory_path() / "serialized-standard.xml";
    minimal_env.serialize(serialize_path);

    std::ifstream serialized_stream(serialize_path);
    std::string actual;
    actual.assign(std::istreambuf_iterator<char>(serialized_stream), std::istreambuf_iterator<char>());

    std::filesystem::path expected_path =
        PROJECT_SOURCE_DIR "/test/data/repos-repomd/repomd-comps-minimal-environment/repodata/comps.xml";
    std::ifstream expected_stream(expected_path);
    std::string expected;
    expected.assign(std::istreambuf_iterator<char>(expected_stream), std::istreambuf_iterator<char>());

    CPPUNIT_ASSERT_EQUAL(expected, actual);
}


void CompsEnvironmentTest::test_solvables() {
    add_repo_repomd("repomd-comps-minimal-environment", false);
    add_repo_repomd("repomd-comps-core", false);
    add_repo_repomd("repomd-comps-core-environment", false);
    add_repo_repomd("repomd-comps-standard");

    EnvironmentQuery q_environments(base);
    auto environments = q_environments.list();
    CPPUNIT_ASSERT_EQUAL((size_t)2, environments.size());

    // Check that environment core is only based on the environment solvables
    // There is a group with id core that has a translation for lang "de", but it shouldn't be used for the environment with id core.
    q_environments.filter_environmentid("core");
    auto core = q_environments.get();
    CPPUNIT_ASSERT_EQUAL(
        std::string("Environment with the same id as an existing group."), core.get_translated_description("de"));
}
