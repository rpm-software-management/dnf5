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

#include "libdnf/comps/comps.hpp"
#include "libdnf/comps/environment/query.hpp"

#include <filesystem>
#include <fstream>


CPPUNIT_TEST_SUITE_REGISTRATION(CompsEnvironmentTest);


using namespace libdnf::comps;


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
    add_repo_repomd("repomd-comps-minimal-environment");
    add_repo_repomd("repomd-comps-custom-environment");
    // load another definiton of the minimal-environment that changes all attributes
    add_repo_repomd("repomd-comps-minimal-environment-v2");

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
    add_repo_repomd("repomd-comps-minimal-environment");
    add_repo_repomd("repomd-comps-custom-environment");
    // load another definiton of the minimal-environment that has all attributes empty
    add_repo_repomd("repomd-comps-minimal-environment-empty");

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
    // load definiton of the minimal-environment that has all attributes empty
    add_repo_repomd("repomd-comps-minimal-environment-empty");
    add_repo_repomd("repomd-comps-custom-environment");
    // load another definiton of the minimal-environment that has all attributes filled
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


void CompsEnvironmentTest::test_merge_different_translations() {
    add_repo_repomd("repomd-comps-minimal-environment");
    // load another definiton of the minimal environment with different set of translations
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


void CompsEnvironmentTest::test_dump() {
    add_repo_repomd("repomd-comps-minimal-environment");

    EnvironmentQuery q_minimal_env(base);
    q_minimal_env.filter_environmentid("minimal-environment");
    auto minimal_env = q_minimal_env.get();

    auto dump_path = std::filesystem::temp_directory_path() / "dumped-standard.xml";
    minimal_env.dump(dump_path);

    std::ifstream dumped_stream(dump_path);
    std::string actual;
    actual.assign(std::istreambuf_iterator<char>(dumped_stream), std::istreambuf_iterator<char>());

    std::filesystem::path expected_path =
        PROJECT_SOURCE_DIR "/test/data/repos-repomd/repomd-comps-minimal-environment/repodata/comps.xml";
    std::ifstream expected_stream(expected_path);
    std::string expected;
    expected.assign(std::istreambuf_iterator<char>(expected_stream), std::istreambuf_iterator<char>());

    CPPUNIT_ASSERT_EQUAL(expected, actual);
}
