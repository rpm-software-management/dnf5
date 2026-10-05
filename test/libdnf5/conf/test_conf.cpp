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
#include "repo/key_auto_import.hpp"

#include <libdnf5/repo/config_repo.hpp>
#include <libdnf5/utils/fs/temp.hpp>

#include <filesystem>
#include <fstream>


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
    // Ensure both pkg_gpgcheck and gpgcheck point to the same underlying OptionBool object

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    // For ConfigMain
    CPPUNIT_ASSERT_EQUAL(&config.get_pkg_gpgcheck_option(), &config.get_gpgcheck_option());

    // For ConfigRepo
    repo::ConfigRepo config_repo(config, "test-repo");
    CPPUNIT_ASSERT_EQUAL(&config_repo.get_pkg_gpgcheck_option(), &config_repo.get_gpgcheck_option());
#pragma GCC diagnostic pop
}

void ConfTest::test_config_auto_import_local_keys() {
    // The option is an opt-in and must default to false.
    CPPUNIT_ASSERT_EQUAL(false, config.get_auto_import_local_keys_option().get_value());

    repo::ConfigRepo config_repo(config, "test-repo");
    CPPUNIT_ASSERT_EQUAL(false, config_repo.get_auto_import_local_keys_option().get_value());

    // The repository option inherits the value set in [main].
    config.get_auto_import_local_keys_option().set(Option::Priority::MAINCONFIG, true);
    CPPUNIT_ASSERT_EQUAL(true, config_repo.get_auto_import_local_keys_option().get_value());

    // A per-repository value overrides the inherited one.
    config_repo.get_auto_import_local_keys_option().set(Option::Priority::REPOCONFIG, false);
    CPPUNIT_ASSERT_EQUAL(false, config_repo.get_auto_import_local_keys_option().get_value());

    // The option is registered under its name on both the main and the
    // repository configuration, so it can be set from configuration files
    // and --setopt.
    config.opt_binds().at("auto_import_local_keys").new_string(Option::Priority::RUNTIME, "0");
    CPPUNIT_ASSERT_EQUAL(false, config.get_auto_import_local_keys_option().get_value());
    config_repo.opt_binds().at("auto_import_local_keys").new_string(Option::Priority::RUNTIME, "1");
    CPPUNIT_ASSERT_EQUAL(true, config_repo.get_auto_import_local_keys_option().get_value());
}

void ConfTest::test_is_key_auto_importable() {
    namespace stdfs = std::filesystem;

    // Layout: keys_dir/key, keys_dir/link -> outside/key, and a sibling
    // directory whose name has keys_dir as a prefix.
    utils::fs::TempDir temp_dir("libdnf_unittest_auto_import");
    const auto keys_dir = temp_dir.get_path() / "rpm-gpg";
    const auto sibling_dir = temp_dir.get_path() / "rpm-gpg-other";
    const auto outside_dir = temp_dir.get_path() / "outside";
    stdfs::create_directories(keys_dir);
    stdfs::create_directories(sibling_dir);
    stdfs::create_directories(outside_dir);
    const auto key = keys_dir / "key";
    const auto sibling_key = sibling_dir / "key";
    const auto outside_key = outside_dir / "key";
    const auto link = keys_dir / "link";
    for (const auto & path : {key, sibling_key, outside_key}) {
        std::ofstream(path) << "not really a key";
    }
    stdfs::create_symlink(outside_key, link);

    repo::ConfigRepo config_repo(config, "test-repo");
    const auto url = [](const stdfs::path & path) { return "file://" + path.string(); };

    // The option is off by default: nothing is imported automatically.
    CPPUNIT_ASSERT_EQUAL(false, repo::is_key_auto_importable(config_repo, url(key), keys_dir));

    config_repo.get_auto_import_local_keys_option().set(Option::Priority::REPOCONFIG, true);

    // A key inside the directory, spelled "file:///path": the one spelling both
    // key readers open as the same file.
    CPPUNIT_ASSERT_EQUAL(true, repo::is_key_auto_importable(config_repo, url(key), keys_dir));
    // A key in a subdirectory of the directory.
    stdfs::create_directories(keys_dir / "sub");
    std::ofstream(keys_dir / "sub" / "key") << "not really a key";
    CPPUNIT_ASSERT_EQUAL(true, repo::is_key_auto_importable(config_repo, url(keys_dir / "sub" / "key"), keys_dir));
    // A symlink inside the directory that resolves to a key inside it.
    const auto alias = keys_dir / "alias";
    stdfs::create_symlink(key, alias);
    CPPUNIT_ASSERT_EQUAL(true, repo::is_key_auto_importable(config_repo, url(alias), keys_dir));

    // Other spellings are left to the confirmation: "file:/path" and a bare
    // path are read differently by the two key readers, a host name is not
    // accepted, and a relative path would be resolved against the working
    // directory.
    CPPUNIT_ASSERT_EQUAL(false, repo::is_key_auto_importable(config_repo, "file:" + key.string(), keys_dir));
    CPPUNIT_ASSERT_EQUAL(false, repo::is_key_auto_importable(config_repo, key.string(), keys_dir));
    CPPUNIT_ASSERT_EQUAL(false, repo::is_key_auto_importable(config_repo, "file://localhost" + key.string(), keys_dir));
    const auto relative_key = stdfs::relative(key, stdfs::current_path());
    CPPUNIT_ASSERT_EQUAL(false, repo::is_key_auto_importable(config_repo, "file://" + relative_key.string(), keys_dir));
    CPPUNIT_ASSERT_EQUAL(false, repo::is_key_auto_importable(config_repo, relative_key.string(), keys_dir));

    // Paths that leave the directory are rejected: "..", a symlink pointing
    // outside, and a sibling directory sharing the prefix.
    CPPUNIT_ASSERT_EQUAL(
        false, repo::is_key_auto_importable(config_repo, url(keys_dir / ".." / "outside" / "key"), keys_dir));
    CPPUNIT_ASSERT_EQUAL(false, repo::is_key_auto_importable(config_repo, url(link), keys_dir));
    CPPUNIT_ASSERT_EQUAL(false, repo::is_key_auto_importable(config_repo, url(sibling_key), keys_dir));
    CPPUNIT_ASSERT_EQUAL(false, repo::is_key_auto_importable(config_repo, url(outside_key), keys_dir));
    // The directory itself is not inside the directory.
    CPPUNIT_ASSERT_EQUAL(false, repo::is_key_auto_importable(config_repo, url(keys_dir), keys_dir));
    // A key that does not exist cannot be trusted.
    CPPUNIT_ASSERT_EQUAL(false, repo::is_key_auto_importable(config_repo, url(keys_dir / "missing"), keys_dir));
    // Remote keys always go through the confirmation.
    CPPUNIT_ASSERT_EQUAL(false, repo::is_key_auto_importable(config_repo, "https://example.com/key", keys_dir));

    // assumeno takes precedence over the option.
    config.get_assumeno_option().set(Option::Priority::RUNTIME, true);
    CPPUNIT_ASSERT_EQUAL(false, repo::is_key_auto_importable(config_repo, url(key), keys_dir));
    config.get_assumeno_option().set(Option::Priority::RUNTIME, false);
    CPPUNIT_ASSERT_EQUAL(true, repo::is_key_auto_importable(config_repo, url(key), keys_dir));
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

void ConfTest::test_gpgcheck_policy_pkg_gpgcheck_no_expand() {
    // pkg_gpgcheck=1 should NOT trigger policy expansion — only gpgcheck=1 should
    ConfigMain main_cfg;
    {
        ConfigParser parser;
        parser.add_section("main");
        parser.set_value("main", "gpgcheck_policy", "full");
        parser.set_value("main", "pkg_gpgcheck", "1");
        main_cfg.load_from_parser(parser, "main", *base->get_vars(), logger);
    }

    CPPUNIT_ASSERT_EQUAL(true, main_cfg.get_pkg_gpgcheck_option().get_value());
    CPPUNIT_ASSERT_EQUAL(false, main_cfg.get_repo_gpgcheck_option().get_value());

    repo::ConfigRepo repo_cfg(main_cfg, "test-repo");
    ConfigParser repo_parser;
    repo_parser.add_section("test-repo");
    repo_parser.set_value("test-repo", "pkg_gpgcheck", "1");
    repo_cfg.load_from_parser(repo_parser, "test-repo", *base->get_vars(), logger);

    CPPUNIT_ASSERT_EQUAL(true, repo_cfg.get_pkg_gpgcheck_option().get_value());
    CPPUNIT_ASSERT_EQUAL(false, repo_cfg.get_repo_gpgcheck_option().get_value());
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
