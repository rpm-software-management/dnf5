/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "test_conf.hpp"

#include "../utils.hpp"

#include "libdnf/rpm/config_repo.hpp"


CPPUNIT_TEST_SUITE_REGISTRATION(ConfTest);

using namespace libdnf;

void ConfTest::setUp() {
    TestCaseFixture::setUp();
    ConfigParser parser;
    parser.read(PROJECT_SOURCE_DIR "/test/libdnf/conf/data/main.conf");
    config.load_from_parser(parser, "main", vars, logger);
}

void ConfTest::test_config_main() {
    CPPUNIT_ASSERT_EQUAL(7, config.debuglevel().get_value());
    CPPUNIT_ASSERT_EQUAL(std::string("hello"), config.persistdir().get_value());
    CPPUNIT_ASSERT_EQUAL(false, config.plugins().get_value());
    std::vector<std::string> pluginpath = {"/foo", "/bar"};
    CPPUNIT_ASSERT_EQUAL(pluginpath, config.pluginpath().get_value());
}

void ConfTest::test_config_repo() {
    rpm::ConfigRepo config_repo(config);
    ConfigParser parser;
    parser.read(PROJECT_SOURCE_DIR "/test/libdnf/conf/data/main.conf");
    vars.load("/", {PROJECT_SOURCE_DIR "/test/libdnf/conf/data/vars"});
    config_repo.load_from_parser(parser, "repo-1", vars, logger);

    std::vector<std::string> baseurl = {"http://example.com/value123", "http://example.com/456"};
    CPPUNIT_ASSERT_EQUAL(baseurl, config_repo.baseurl().get_value());
}
