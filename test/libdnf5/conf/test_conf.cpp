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

namespace {

libdnf5::ConfigMain copy_construct_config_main(const libdnf5::ConfigMain & config) {
    return libdnf5::ConfigMain{config};
}

libdnf5::ConfigMain assign_config_main(const libdnf5::ConfigMain & config) {
    libdnf5::ConfigMain config_copy;
    config_copy = config;
    return config_copy;
}

}  // namespace

void test_copy_config_main(std::function<libdnf5::ConfigMain(const libdnf5::ConfigMain &)> copy_function) {
    libdnf5::ConfigMain config;

    config.get_assumeyes_option().set(libdnf5::Option::Priority::MAINCONFIG, false);
    config.get_debuglevel_option().set(libdnf5::Option::Priority::RUNTIME, 7);
    config.get_allow_downgrade_option().set(libdnf5::Option::Priority::RUNTIME, false);
    config.get_destdir_option().set(libdnf5::Option::Priority::RUNTIME, "foobar");

    auto config_copy = copy_function(config);

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

    CPPUNIT_ASSERT_EQUAL(0ul, config_copy.get_excludepkgs_option().get_value().size());
    CPPUNIT_ASSERT_EQUAL(0ul, config.get_excludepkgs_option().get_value().size());

    config_copy.get_excludepkgs_option().add_item(libdnf5::Option::Priority::RUNTIME, "abc");

    CPPUNIT_ASSERT_EQUAL(1ul, config_copy.get_excludepkgs_option().get_value().size());
    CPPUNIT_ASSERT_EQUAL(0ul, config.get_excludepkgs_option().get_value().size());
}

void ConfTest::test_config_assign_config_main() {
    test_copy_config_main(assign_config_main);
}

void ConfTest::test_config_copy_construct_config_main() {
    test_copy_config_main(copy_construct_config_main);
}
