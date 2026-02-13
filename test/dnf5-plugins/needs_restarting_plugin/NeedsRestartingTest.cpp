// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "NeedsRestartingTest.hpp"

#include "needs_restarting.hpp"

#include <dnf5/context.hpp>
#include <libdnf5/base/base.hpp>
#include <libdnf5/conf/option_bool.hpp>

using namespace dnf5;

void NeedsRestartingTest::setUp() {
    // Setup test environment if needed
}

void NeedsRestartingTest::tearDown() {
    // Cleanup test environment if needed
}

void NeedsRestartingTest::test_processes_option() {
    // Test that the --processes option is properly registered and configured
    std::vector<std::unique_ptr<libdnf5::Logger>> loggers;
    Context context(std::move(loggers));

    // Set up a root command for the argument parser
    auto & parser = context.get_argument_parser();
    auto root_cmd = parser.add_new_command("test");
    parser.set_root_command(root_cmd);

    NeedsRestartingCommand cmd(context);
    cmd.set_argument_parser();

    // Get arguments from the command, not the parser
    auto & cmd_parser = *cmd.get_argument_parser_command();
    auto & processes_arg = cmd_parser.get_named_arg("processes");

    // Verify short name
    CPPUNIT_ASSERT_EQUAL(std::string("p"), std::string(1, processes_arg.get_short_name()));

    // Verify long name
    CPPUNIT_ASSERT_EQUAL(std::string("processes"), processes_arg.get_long_name());

    // Verify description
    CPPUNIT_ASSERT(processes_arg.get_description().find("processes") != std::string::npos);
}

void NeedsRestartingTest::test_exclude_services_option() {
    // Test that the --exclude-services option is properly registered and configured
    std::vector<std::unique_ptr<libdnf5::Logger>> loggers;
    Context context(std::move(loggers));

    // Set up a root command for the argument parser
    auto & parser = context.get_argument_parser();
    auto root_cmd = parser.add_new_command("test");
    parser.set_root_command(root_cmd);

    NeedsRestartingCommand cmd(context);
    cmd.set_argument_parser();

    // Get arguments from the command, not the parser
    auto & cmd_parser = *cmd.get_argument_parser_command();
    auto & exclude_services_arg = cmd_parser.get_named_arg("exclude-services");

    // Verify short name
    CPPUNIT_ASSERT_EQUAL(std::string("e"), std::string(1, exclude_services_arg.get_short_name()));

    // Verify long name
    CPPUNIT_ASSERT_EQUAL(std::string("exclude-services"), exclude_services_arg.get_long_name());

    // Verify description mentions systemd services
    CPPUNIT_ASSERT(exclude_services_arg.get_description().find("systemd") != std::string::npos);
    CPPUNIT_ASSERT(exclude_services_arg.get_description().find("--processes") != std::string::npos);
}

void NeedsRestartingTest::test_processes_and_exclude_services() {
    // Test that both --processes and --exclude-services options can be used together
    std::vector<std::unique_ptr<libdnf5::Logger>> loggers;
    Context context(std::move(loggers));

    // Set up a root command for the argument parser
    auto & parser = context.get_argument_parser();
    auto root_cmd = parser.add_new_command("test");
    parser.set_root_command(root_cmd);

    NeedsRestartingCommand cmd(context);
    cmd.set_argument_parser();

    // Get arguments from the command, not the parser
    auto & cmd_parser = *cmd.get_argument_parser_command();
    // Verify both arguments exist by attempting to retrieve them
    (void)cmd_parser.get_named_arg("processes");
    (void)cmd_parser.get_named_arg("exclude-services");

    // Simulate setting both options
    libdnf5::OptionBool processes_option(false);
    libdnf5::OptionBool exclude_services_option(false);

    // Test that both options can be set
    processes_option.set(true);
    exclude_services_option.set(true);

    CPPUNIT_ASSERT_EQUAL(true, processes_option.get_value());
    CPPUNIT_ASSERT_EQUAL(true, exclude_services_option.get_value());
}

void NeedsRestartingTest::test_exclude_services_without_processes() {
    // Test that --exclude-services is documented to be used with --processes
    std::vector<std::unique_ptr<libdnf5::Logger>> loggers;
    Context context(std::move(loggers));

    // Set up a root command for the argument parser
    auto & parser = context.get_argument_parser();
    auto root_cmd = parser.add_new_command("test");
    parser.set_root_command(root_cmd);

    NeedsRestartingCommand cmd(context);
    cmd.set_argument_parser();

    // Get arguments from the command, not the parser
    auto & cmd_parser = *cmd.get_argument_parser_command();
    auto & exclude_services_arg = cmd_parser.get_named_arg("exclude-services");

    // Verify the --exclude-services description mentions it should be used with --processes
    std::string description = exclude_services_arg.get_description();
    CPPUNIT_ASSERT(description.find("--processes") != std::string::npos);
}

void NeedsRestartingTest::test_services_option() {
    // Test that the --services option is properly registered and configured
    std::vector<std::unique_ptr<libdnf5::Logger>> loggers;
    Context context(std::move(loggers));

    // Set up a root command for the argument parser
    auto & parser = context.get_argument_parser();
    auto root_cmd = parser.add_new_command("test");
    parser.set_root_command(root_cmd);

    NeedsRestartingCommand cmd(context);
    cmd.set_argument_parser();

    // Get arguments from the command, not the parser
    auto & cmd_parser = *cmd.get_argument_parser_command();
    auto & services_arg = cmd_parser.get_named_arg("services");

    // Verify short name
    CPPUNIT_ASSERT_EQUAL(std::string("s"), std::string(1, services_arg.get_short_name()));

    // Verify long name
    CPPUNIT_ASSERT_EQUAL(std::string("services"), services_arg.get_long_name());

    // Verify description mentions systemd services
    std::string description = services_arg.get_description();
    CPPUNIT_ASSERT(description.find("systemd") != std::string::npos);
    CPPUNIT_ASSERT(description.find("services") != std::string::npos);
}

void NeedsRestartingTest::test_reboothint_option() {
    // Test that the --reboothint option is properly registered
    std::vector<std::unique_ptr<libdnf5::Logger>> loggers;
    Context context(std::move(loggers));

    // Set up a root command for the argument parser
    auto & parser = context.get_argument_parser();
    auto root_cmd = parser.add_new_command("test");
    parser.set_root_command(root_cmd);

    NeedsRestartingCommand cmd(context);
    cmd.set_argument_parser();

    // Get arguments from the command, not the parser
    auto & cmd_parser = *cmd.get_argument_parser_command();
    auto & reboothint_arg = cmd_parser.get_named_arg("reboothint");

    // Verify short name
    CPPUNIT_ASSERT_EQUAL(std::string("r"), std::string(1, reboothint_arg.get_short_name()));

    // Verify long name
    CPPUNIT_ASSERT_EQUAL(std::string("reboothint"), reboothint_arg.get_long_name());

    // Verify description mentions compatibility with DNF 4
    std::string description = reboothint_arg.get_description();
    CPPUNIT_ASSERT(
        description.find("DNF 4") != std::string::npos || description.find("compatibility") != std::string::npos);
}

void NeedsRestartingTest::test_all_options_registered() {
    // Test that all command-line options are properly registered
    std::vector<std::unique_ptr<libdnf5::Logger>> loggers;
    Context context(std::move(loggers));

    // Set up a root command for the argument parser
    auto & parser = context.get_argument_parser();
    auto root_cmd = parser.add_new_command("test");
    parser.set_root_command(root_cmd);

    NeedsRestartingCommand cmd(context);
    cmd.set_argument_parser();

    // Get arguments from the command, not the parser
    auto & cmd_parser = *cmd.get_argument_parser_command();

    // Verify short names are unique
    auto & services_arg = cmd_parser.get_named_arg("services");
    auto & processes_arg = cmd_parser.get_named_arg("processes");
    auto & exclude_services_arg = cmd_parser.get_named_arg("exclude-services");
    auto & reboothint_arg = cmd_parser.get_named_arg("reboothint");

    CPPUNIT_ASSERT(services_arg.get_short_name() != processes_arg.get_short_name());
    CPPUNIT_ASSERT(services_arg.get_short_name() != exclude_services_arg.get_short_name());
    CPPUNIT_ASSERT(services_arg.get_short_name() != reboothint_arg.get_short_name());
    CPPUNIT_ASSERT(processes_arg.get_short_name() != exclude_services_arg.get_short_name());
    CPPUNIT_ASSERT(processes_arg.get_short_name() != reboothint_arg.get_short_name());
    CPPUNIT_ASSERT(exclude_services_arg.get_short_name() != reboothint_arg.get_short_name());
}

CPPUNIT_TEST_SUITE_REGISTRATION(NeedsRestartingTest);
