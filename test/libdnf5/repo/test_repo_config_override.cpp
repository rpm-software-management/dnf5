// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "test_repo_config_override.hpp"

#include <libdnf5/conf/config_parser.hpp>
#include <libdnf5/conf/const.hpp>

#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <string>


CPPUNIT_TEST_SUITE_REGISTRATION(RepoConfigOverrideTest);

namespace fs = std::filesystem;


void RepoConfigOverrideTest::test_override_file_path() {
    // BaseTestCase::setUp() already sets installroot to temp_dir/installroot.
    // Verify the path is rooted under that installroot.
    auto installroot = fs::path(base.get_config().get_installroot_option().get_value());
    auto path = repo_sack->get_user_repos_override_file_path();

    fs::path expected =
        installroot / fs::path(libdnf5::REPOS_OVERRIDE_DIR).relative_path() / libdnf5::USER_REPOS_OVERRIDE_FILENAME;
    CPPUNIT_ASSERT_EQUAL(expected.string(), path.string());
}


void RepoConfigOverrideTest::test_set_overrides() {
    auto override_path = repo_sack->get_user_repos_override_file_path();

    std::map<std::string, std::map<std::string, std::string>> overrides;
    overrides["testrepo"] = {{"enabled", "0"}};
    repo_sack->override_repos_configuration(overrides);

    CPPUNIT_ASSERT(fs::exists(override_path));

    libdnf5::ConfigParser parser;
    parser.read(override_path);
    CPPUNIT_ASSERT(parser.has_section("testrepo"));
    CPPUNIT_ASSERT_EQUAL(std::string("0"), parser.get_value("testrepo", "enabled"));
}


void RepoConfigOverrideTest::test_set_overrides_updates_existing() {
    std::map<std::string, std::map<std::string, std::string>> overrides;
    overrides["testrepo"] = {{"enabled", "1"}, {"gpgcheck", "0"}};
    repo_sack->override_repos_configuration(overrides);

    // Update enabled, leave gpgcheck untouched
    overrides.clear();
    overrides["testrepo"] = {{"enabled", "0"}};
    repo_sack->override_repos_configuration(overrides);

    auto override_path = repo_sack->get_user_repos_override_file_path();
    libdnf5::ConfigParser parser;
    parser.read(override_path);

    CPPUNIT_ASSERT_EQUAL(std::string("0"), parser.get_value("testrepo", "enabled"));
    CPPUNIT_ASSERT_EQUAL(std::string("0"), parser.get_value("testrepo", "gpgcheck"));
}


void RepoConfigOverrideTest::test_set_overrides_multiple_repos() {
    std::map<std::string, std::map<std::string, std::string>> overrides;
    overrides["repo-a"] = {{"enabled", "1"}};
    overrides["repo-b"] = {{"enabled", "0"}, {"priority", "10"}};
    overrides["repo-c"] = {{"gpgcheck", "1"}};
    repo_sack->override_repos_configuration(overrides);

    auto override_path = repo_sack->get_user_repos_override_file_path();
    libdnf5::ConfigParser parser;
    parser.read(override_path);

    CPPUNIT_ASSERT(parser.has_section("repo-a"));
    CPPUNIT_ASSERT(parser.has_section("repo-b"));
    CPPUNIT_ASSERT(parser.has_section("repo-c"));
    CPPUNIT_ASSERT_EQUAL(std::string("1"), parser.get_value("repo-a", "enabled"));
    CPPUNIT_ASSERT_EQUAL(std::string("0"), parser.get_value("repo-b", "enabled"));
    CPPUNIT_ASSERT_EQUAL(std::string("10"), parser.get_value("repo-b", "priority"));
    CPPUNIT_ASSERT_EQUAL(std::string("1"), parser.get_value("repo-c", "gpgcheck"));
}


void RepoConfigOverrideTest::test_remove_overrides() {
    std::map<std::string, std::map<std::string, std::string>> overrides;
    overrides["testrepo"] = {{"enabled", "0"}, {"gpgcheck", "1"}, {"priority", "5"}};
    repo_sack->override_repos_configuration(overrides);

    std::map<std::string, std::set<std::string>> removals;
    removals["testrepo"] = {"gpgcheck"};
    repo_sack->override_repos_configuration({}, removals);

    auto override_path = repo_sack->get_user_repos_override_file_path();
    libdnf5::ConfigParser parser;
    parser.read(override_path);

    CPPUNIT_ASSERT(parser.has_section("testrepo"));
    CPPUNIT_ASSERT_EQUAL(std::string("0"), parser.get_value("testrepo", "enabled"));
    CPPUNIT_ASSERT(!parser.has_option("testrepo", "gpgcheck"));
    CPPUNIT_ASSERT_EQUAL(std::string("5"), parser.get_value("testrepo", "priority"));
}


void RepoConfigOverrideTest::test_remove_overrides_cleans_empty_sections() {
    std::map<std::string, std::map<std::string, std::string>> overrides;
    overrides["testrepo"] = {{"enabled", "0"}};
    repo_sack->override_repos_configuration(overrides);

    // Remove the only key — section should be removed
    std::map<std::string, std::set<std::string>> removals;
    removals["testrepo"] = {"enabled"};
    repo_sack->override_repos_configuration({}, removals);

    auto override_path = repo_sack->get_user_repos_override_file_path();
    libdnf5::ConfigParser parser;
    parser.read(override_path);
    CPPUNIT_ASSERT(!parser.has_section("testrepo"));
}


void RepoConfigOverrideTest::test_set_and_remove_overrides() {
    // Set up initial data
    std::map<std::string, std::map<std::string, std::string>> overrides;
    overrides["repo-a"] = {{"enabled", "1"}, {"gpgcheck", "0"}};
    overrides["repo-b"] = {{"priority", "10"}};
    repo_sack->override_repos_configuration(overrides);

    // In one call: set new value on repo-a, remove key from repo-a
    overrides.clear();
    overrides["repo-a"] = {{"baseurl", "http://example.com"}};
    std::map<std::string, std::set<std::string>> removals;
    removals["repo-a"] = {"gpgcheck"};
    repo_sack->override_repos_configuration(overrides, removals);

    auto override_path = repo_sack->get_user_repos_override_file_path();
    libdnf5::ConfigParser parser;
    parser.read(override_path);

    CPPUNIT_ASSERT_EQUAL(std::string("1"), parser.get_value("repo-a", "enabled"));
    CPPUNIT_ASSERT_EQUAL(std::string("http://example.com"), parser.get_value("repo-a", "baseurl"));
    CPPUNIT_ASSERT(!parser.has_option("repo-a", "gpgcheck"));
    CPPUNIT_ASSERT_EQUAL(std::string("10"), parser.get_value("repo-b", "priority"));
}


void RepoConfigOverrideTest::test_no_changes_skips_write() {
    // Call with only removals on a non-existent section — nothing to remove,
    // file should not be created.
    auto override_path = repo_sack->get_user_repos_override_file_path();

    std::map<std::string, std::set<std::string>> removals;
    removals["nonexistent"] = {"somekey"};
    repo_sack->override_repos_configuration({}, removals);

    CPPUNIT_ASSERT(!fs::exists(override_path));
}
