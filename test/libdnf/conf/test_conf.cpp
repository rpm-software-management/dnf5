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

#include "utils.hpp"

#include "libdnf/repo/config_repo.hpp"


CPPUNIT_TEST_SUITE_REGISTRATION(ConfTest);

using namespace libdnf;

void ConfTest::setUp() {
    TestCaseFixture::setUp();
    base = get_preconfigured_base();
    ConfigParser parser;
    parser.read(PROJECT_SOURCE_DIR "/test/libdnf/conf/data/main.conf");
    config.load_from_parser(parser, "main", *base->get_vars(), logger);
}

void ConfTest::test_config_main() {
    CPPUNIT_ASSERT_EQUAL(7, config.debuglevel().get_value());
    CPPUNIT_ASSERT_EQUAL(std::string("hello"), config.persistdir().get_value());
    CPPUNIT_ASSERT_EQUAL(false, config.plugins().get_value());
    std::string pluginpath = "/foo";
    CPPUNIT_ASSERT_EQUAL(pluginpath, config.pluginpath().get_value());
}

void ConfTest::test_config_repo() {
    repo::ConfigRepo config_repo(config, "test-repo");
    ConfigParser parser;
    parser.read(PROJECT_SOURCE_DIR "/test/libdnf/conf/data/main.conf");
    base->get_config().varsdir().set(std::vector<std::string>{PROJECT_SOURCE_DIR "/test/libdnf/conf/data/vars"});
    base->setup();
    config_repo.load_from_parser(parser, "repo-1", *base->get_vars(), logger);

    std::vector<std::string> baseurl = {"http://example.com/value123", "http://example.com/456"};
    CPPUNIT_ASSERT_EQUAL(baseurl, config_repo.baseurl().get_value());
}
