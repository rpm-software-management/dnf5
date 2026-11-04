// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of DNF5: https://github.com/rpm-software-management/dnf5/
//
// DNF5 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// DNF5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DNF5.  If not, see <https://www.gnu.org/licenses/>.

#include "DropInConfig.hpp"

#include "config_automatic.hpp"

#include <libdnf5/conf/config_parser.hpp>
#include <libdnf5/utils/fs/utils.hpp>

#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

void DropInConfigTest::test_drop_in_config_loading() {
    // Create a temporary directory for test configuration files
    auto temp_dir = fs::temp_directory_path() / "dnf5-test-dropin";
    fs::create_directories(temp_dir);
    auto conf_d_dir = temp_dir / "automatic.conf.d";
    fs::create_directories(conf_d_dir);

    // Create drop-in configuration files
    std::ofstream file1(conf_d_dir / "10-test.conf");
    file1 << "[commands]\n";
    file1 << "random_sleep = 42\n";
    file1.close();

    std::ofstream file2(conf_d_dir / "20-test.conf");
    file2 << "[emitters]\n";
    file2 << "emit_no_updates = yes\n";
    file2.close();

    // Load configuration files using libdnf5 utilities
    std::vector<fs::path> conf_dirs{conf_d_dir};
    std::vector<fs::path> conf_files = libdnf5::utils::fs::create_sorted_file_list(conf_dirs, ".conf");

    // Verify files are loaded in alphabetical order
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Should load 2 configuration files", size_t(2), conf_files.size());
    CPPUNIT_ASSERT_MESSAGE("First file should be 10-test.conf", conf_files[0].filename().string() == "10-test.conf");
    CPPUNIT_ASSERT_MESSAGE("Second file should be 20-test.conf", conf_files[1].filename().string() == "20-test.conf");

    // Clean up
    fs::remove_all(temp_dir);
}

void DropInConfigTest::test_drop_in_config_override() {
    // Create a temporary directory for test configuration files
    auto temp_dir = fs::temp_directory_path() / "dnf5-test-dropin-override";
    fs::create_directories(temp_dir);
    auto conf_d_dir = temp_dir / "automatic.conf.d";
    fs::create_directories(conf_d_dir);

    // Create drop-in configuration files with overlapping values
    std::ofstream file1(conf_d_dir / "10-first.conf");
    file1 << "[commands]\n";
    file1 << "random_sleep = 10\n";
    file1 << "download_updates = no\n";
    file1.close();

    std::ofstream file2(conf_d_dir / "20-second.conf");
    file2 << "[commands]\n";
    file2 << "random_sleep = 20\n";
    file2.close();

    // Load and parse configuration files
    auto base = get_preconfigured_base();
    auto & logger = *base->get_logger();
    auto & vars = *base->get_vars();

    // Parse first file
    libdnf5::ConfigParser parser1;
    parser1.read((conf_d_dir / "10-first.conf").string());
    dnf5::ConfigAutomaticCommands config1;
    config1.load_from_parser(parser1, "commands", vars, logger, libdnf5::Option::Priority::AUTOMATICCONFIG);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "First config random_sleep should be 10", uint32_t(10), config1.random_sleep.get_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "First config download_updates should be false", false, config1.download_updates.get_value());

    // Parse second file - should override random_sleep but not download_updates
    libdnf5::ConfigParser parser2;
    parser2.read((conf_d_dir / "20-second.conf").string());
    dnf5::ConfigAutomaticCommands config2;
    config2.load_from_parser(parser2, "commands", vars, logger, libdnf5::Option::Priority::AUTOMATICCONFIG);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Second config random_sleep should be 20", uint32_t(20), config2.random_sleep.get_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Second config download_updates should keep default", true, config2.download_updates.get_value());

    // Clean up
    fs::remove_all(temp_dir);
}

void DropInConfigTest::test_etc_drop_in_precedence() {
    // Create temporary directories simulating both /usr/share and /etc hierarchy
    auto temp_dir = fs::temp_directory_path() / "dnf5-test-etc-precedence";
    fs::create_directories(temp_dir);

    auto usr_dir = temp_dir / "usr_share_dnf5";
    auto etc_dir = temp_dir / "etc_dnf";
    fs::create_directories(usr_dir / "automatic.conf.d");
    fs::create_directories(etc_dir / "automatic.conf.d");

    // Create system defaults in /usr/share
    std::ofstream usr_file(usr_dir / "automatic.conf.d" / "10-system.conf");
    usr_file << "[commands]\n";
    usr_file << "random_sleep = 100\n";
    usr_file << "download_updates = yes\n";
    usr_file.close();

    // Create local override in /etc/dnf
    std::ofstream etc_file(etc_dir / "automatic.conf.d" / "10-local.conf");
    etc_file << "[commands]\n";
    etc_file << "random_sleep = 200\n";
    etc_file.close();

    // Load and parse configuration files
    auto base = get_preconfigured_base();
    auto & logger = *base->get_logger();
    auto & vars = *base->get_vars();
    dnf5::ConfigAutomaticCommands final_config;

    // First: Load system defaults
    libdnf5::ConfigParser parser1;
    parser1.read((usr_dir / "automatic.conf.d" / "10-system.conf").string());
    final_config.load_from_parser(parser1, "commands", vars, logger, libdnf5::Option::Priority::AUTOMATICCONFIG);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "After system config, random_sleep should be 100", uint32_t(100), final_config.random_sleep.get_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "After system config, download_updates should be yes", true, final_config.download_updates.get_value());

    // Then: Load /etc override
    libdnf5::ConfigParser parser2;
    parser2.read((etc_dir / "automatic.conf.d" / "10-local.conf").string());
    final_config.load_from_parser(parser2, "commands", vars, logger, libdnf5::Option::Priority::AUTOMATICCONFIG);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "/etc override should set random_sleep to 200", uint32_t(200), final_config.random_sleep.get_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "/etc override shouldn't change unspecified download_updates", true, final_config.download_updates.get_value());

    // Clean up
    fs::remove_all(temp_dir);
}

CPPUNIT_TEST_SUITE_REGISTRATION(DropInConfigTest);
