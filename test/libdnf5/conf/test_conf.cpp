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


#include "test_conf.hpp"

#include "../shared/utils.hpp"

#include <libdnf5/repo/config_repo.hpp>


CPPUNIT_TEST_SUITE_REGISTRATION(ConfTest);

using namespace libdnf5;

void ConfTest::setUp() {
    TestCaseFixture::setUp();
    base = get_preconfigured_base();
    ConfigParser parser;
    parser.read(PROJECT_SOURCE_DIR "/test/libdnf5/conf/data/main.conf");
    config.load_from_parser(parser, "main", *base->get_vars(), logger);
}

void ConfTest::test_config_main() {
    CPPUNIT_ASSERT_EQUAL(7, config.get_debuglevel_option().get_value());
    CPPUNIT_ASSERT_EQUAL(std::string("hello"), config.get_persistdir_option().get_value());
    CPPUNIT_ASSERT_EQUAL(false, config.get_plugins_option().get_value());
    std::string pluginpath = "/foo";
    CPPUNIT_ASSERT_EQUAL(pluginpath, config.get_pluginpath_option().get_value());
}

void ConfTest::test_config_repo() {
    repo::ConfigRepo config_repo(config, "test-repo");
    ConfigParser parser;
    parser.read(PROJECT_SOURCE_DIR "/test/libdnf5/conf/data/main.conf");
    base->get_config().get_varsdir_option().set(
        std::vector<std::string>{PROJECT_SOURCE_DIR "/test/libdnf5/conf/data/vars"});
    base->setup();
    config_repo.load_from_parser(parser, "repo-1", *base->get_vars(), logger);

    std::vector<std::string> baseurl = {"http://example.com/value123", "http://example.com/456"};
    CPPUNIT_ASSERT_EQUAL(baseurl, config_repo.get_baseurl_option().get_value());
}

void ConfTest::test_config_pkg_gpgcheck() {
    // gpgcheck and pkg_gpgcheck are now separate options (gpgcheck is expanded via gpgcheck_policy)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    // For ConfigMain: gpgcheck and pkg_gpgcheck are separate objects
    CPPUNIT_ASSERT(&config.get_pkg_gpgcheck_option() != &config.get_gpgcheck_option());
#pragma GCC diagnostic pop
}

void ConfTest::test_gpgcheck_policy_legacy() {
    ConfigMain cfg;
    ConfigParser parser;
    parser.add_section("main");
    parser.set_value("main", "gpgcheck_policy", "legacy");
    parser.set_value("main", "gpgcheck", "1");
    cfg.load_from_parser(parser, "main", *base->get_vars(), logger);

    CPPUNIT_ASSERT_EQUAL(true, cfg.get_pkg_gpgcheck_option().get_value());
    CPPUNIT_ASSERT_EQUAL(false, cfg.get_repo_gpgcheck_option().get_value());
    CPPUNIT_ASSERT_EQUAL(false, cfg.get_localpkg_gpgcheck_option().get_value());
}

void ConfTest::test_gpgcheck_policy_full() {
    ConfigMain cfg;
    ConfigParser parser;
    parser.add_section("main");
    parser.set_value("main", "gpgcheck_policy", "full");
    parser.set_value("main", "gpgcheck", "1");
    cfg.load_from_parser(parser, "main", *base->get_vars(), logger);

    CPPUNIT_ASSERT_EQUAL(true, cfg.get_pkg_gpgcheck_option().get_value());
    CPPUNIT_ASSERT_EQUAL(true, cfg.get_repo_gpgcheck_option().get_value());
    CPPUNIT_ASSERT_EQUAL(false, cfg.get_localpkg_gpgcheck_option().get_value());
}

void ConfTest::test_gpgcheck_policy_all() {
    ConfigMain cfg;
    ConfigParser parser;
    parser.add_section("main");
    parser.set_value("main", "gpgcheck_policy", "all");
    parser.set_value("main", "gpgcheck", "1");
    cfg.load_from_parser(parser, "main", *base->get_vars(), logger);

    CPPUNIT_ASSERT_EQUAL(true, cfg.get_pkg_gpgcheck_option().get_value());
    CPPUNIT_ASSERT_EQUAL(true, cfg.get_repo_gpgcheck_option().get_value());
    CPPUNIT_ASSERT_EQUAL(true, cfg.get_localpkg_gpgcheck_option().get_value());
}

void ConfTest::test_gpgcheck_policy_explicit_override() {
    ConfigMain cfg;
    ConfigParser parser;
    parser.add_section("main");
    parser.set_value("main", "gpgcheck_policy", "full");
    parser.set_value("main", "gpgcheck", "1");
    parser.set_value("main", "repo_gpgcheck", "0");
    cfg.load_from_parser(parser, "main", *base->get_vars(), logger);

    CPPUNIT_ASSERT_EQUAL(true, cfg.get_pkg_gpgcheck_option().get_value());
    CPPUNIT_ASSERT_EQUAL(false, cfg.get_repo_gpgcheck_option().get_value());
    CPPUNIT_ASSERT_EQUAL(false, cfg.get_localpkg_gpgcheck_option().get_value());
}

void ConfTest::test_gpgcheck_policy_repo() {
    ConfigMain main_cfg;
    {
        ConfigParser parser;
        parser.add_section("main");
        parser.set_value("main", "gpgcheck_policy", "full");
        main_cfg.load_from_parser(parser, "main", *base->get_vars(), logger);
    }

    repo::ConfigRepo repo_cfg(main_cfg, "test-repo");
    ConfigParser repo_parser;
    repo_parser.add_section("test-repo");
    repo_parser.set_value("test-repo", "gpgcheck", "1");
    repo_cfg.load_from_parser(repo_parser, "test-repo", *base->get_vars(), logger);

    CPPUNIT_ASSERT_EQUAL(true, repo_cfg.get_pkg_gpgcheck_option().get_value());
    CPPUNIT_ASSERT_EQUAL(true, repo_cfg.get_repo_gpgcheck_option().get_value());

    repo::ConfigRepo repo_cfg2(main_cfg, "test-repo-2");
    ConfigParser repo_parser2;
    repo_parser2.add_section("test-repo-2");
    repo_parser2.set_value("test-repo-2", "gpgcheck", "1");
    repo_parser2.set_value("test-repo-2", "repo_gpgcheck", "0");
    repo_cfg2.load_from_parser(repo_parser2, "test-repo-2", *base->get_vars(), logger);

    CPPUNIT_ASSERT_EQUAL(true, repo_cfg2.get_pkg_gpgcheck_option().get_value());
    CPPUNIT_ASSERT_EQUAL(false, repo_cfg2.get_repo_gpgcheck_option().get_value());
}

void ConfTest::test_config_load_from_config() {
    libdnf5::ConfigMain config;

    config.get_assumeyes_option().set(libdnf5::Option::Priority::MAINCONFIG, false);
    config.get_debuglevel_option().set(libdnf5::Option::Priority::RUNTIME, 7);
    config.get_allow_downgrade_option().set(libdnf5::Option::Priority::RUNTIME, false);
    config.get_destdir_option().set(libdnf5::Option::Priority::RUNTIME, "foobar");

    libdnf5::ConfigMain config_copy;
    config_copy.load_from_config(config);

    CPPUNIT_ASSERT_EQUAL(false, config_copy.get_allow_downgrade_option().get_value());
    CPPUNIT_ASSERT_EQUAL(std::string{"foobar"}, config_copy.get_destdir_option().get_value());

    CPPUNIT_ASSERT_EQUAL(libdnf5::Option::Priority::MAINCONFIG, config.get_assumeyes_option().get_priority());
    CPPUNIT_ASSERT_EQUAL(false, config.get_assumeyes_option().get_value());

    CPPUNIT_ASSERT_EQUAL(libdnf5::Option::Priority::MAINCONFIG, config_copy.get_assumeyes_option().get_priority());
    CPPUNIT_ASSERT_EQUAL(false, config_copy.get_assumeyes_option().get_value());

    config_copy.get_assumeyes_option().set(libdnf5::Option::Priority::RUNTIME, true);

    CPPUNIT_ASSERT_EQUAL(libdnf5::Option::Priority::MAINCONFIG, config.get_assumeyes_option().get_priority());
    CPPUNIT_ASSERT_EQUAL(false, config.get_assumeyes_option().get_value());

    CPPUNIT_ASSERT_EQUAL(libdnf5::Option::Priority::RUNTIME, config_copy.get_assumeyes_option().get_priority());
    CPPUNIT_ASSERT_EQUAL(true, config_copy.get_assumeyes_option().get_value());

    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), config_copy.get_excludepkgs_option().get_value().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), config.get_excludepkgs_option().get_value().size());

    config_copy.get_excludepkgs_option().add_item(libdnf5::Option::Priority::RUNTIME, "abc");

    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), config_copy.get_excludepkgs_option().get_value().size());
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), config.get_excludepkgs_option().get_value().size());
}
