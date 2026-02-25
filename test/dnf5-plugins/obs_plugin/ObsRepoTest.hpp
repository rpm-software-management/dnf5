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


#ifndef DNF5_TEST_OBS_REPO_HPP
#define DNF5_TEST_OBS_REPO_HPP

// Note 1
#include "../../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>

class ObsRepoTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(ObsRepoTest);
    CPPUNIT_TEST(test_list_sth);
    CPPUNIT_TEST(test_enable_no_hubspec);
    CPPUNIT_TEST(test_enable_short_hubspec);
    CPPUNIT_TEST(test_enable_full_hubspec);
    CPPUNIT_TEST(test_enable_unofficial_short_hubspec);
    CPPUNIT_TEST(test_enable_unofficial_full_hubspec);
    CPPUNIT_TEST(test_enable_html_fallback);
    CPPUNIT_TEST(test_disable);
    CPPUNIT_TEST(test_remove);
    CPPUNIT_TEST_SUITE_END();

    std::filesystem::path installroot;
    std::filesystem::path confdir;
    std::filesystem::path repodir;

    void prepare_env();
    void add_obs_config(const std::string config_filename);
    void install_repofile(const std::string & name);
    void install_configfile(const std::string name);
    void reload_repofiles();
    void run_test_enable(
        const std::string & project_repo_spec,
        const std::string & repo_filename,
        const std::string & config_filename = "");

public:
    void test_list_sth();
    void test_enable_no_hubspec();
    void test_enable_short_hubspec();
    void test_enable_full_hubspec();
    void test_enable_unofficial_short_hubspec();
    void test_enable_unofficial_full_hubspec();
    void test_enable_html_fallback();
    void test_disable();
    void test_remove();
};

#endif  // DNF5_TEST_OBS_REPO_HPP
