/*
Copyright (C) 2020-2021 Red Hat, Inc.

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


#include "test_goal.hpp"

#include "libdnf/base/goal.hpp"

#include <libdnf/rpm/solv_query.hpp>


CPPUNIT_TEST_SUITE_REGISTRATION(BaseGoalTest);

void BaseGoalTest::setUp() {
    RepoFixture::setUp();
    add_repo_repomd("repomd-repo1");
}

void BaseGoalTest::test_install() {
    libdnf::Goal goal(base.get());
    goal.add_rpm_install("pkg", {}, true, {});
    goal.resolve();
    auto install_set = goal.list_rpm_installs();
    auto reinstall_set = goal.list_rpm_reinstalls();
    auto upgrade_set = goal.list_rpm_upgrades();
    auto downgrade_set = goal.list_rpm_downgrades();
    auto remove_set = goal.list_rpm_removes();
    auto obsoleted_set = goal.list_rpm_obsoleted();
    CPPUNIT_ASSERT_EQUAL(1lu, install_set.size());
    CPPUNIT_ASSERT_EQUAL(std::string("pkg-0:1.2-3.x86_64"), install_set[0].get_full_nevra());
    CPPUNIT_ASSERT_EQUAL(0lu, reinstall_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, upgrade_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, downgrade_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, remove_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, obsoleted_set.size());
}

void BaseGoalTest::test_install_from_cmdline() {
    libdnf::Goal goal(base.get());
    std::filesystem::path rpm_path =
        PROJECT_BINARY_DIR "/test/data/cmdline-rpms/noarch/cmdline-1.2-3.noarch.rpm";
    auto cmd_pkg = sack->add_cmdline_package(rpm_path, false);
    goal.add_rpm_install(cmd_pkg, true);
    goal.resolve();
    auto install_set = goal.list_rpm_installs();
    auto reinstall_set = goal.list_rpm_reinstalls();
    auto upgrade_set = goal.list_rpm_upgrades();
    auto downgrade_set = goal.list_rpm_downgrades();
    auto remove_set = goal.list_rpm_removes();
    auto obsoleted_set = goal.list_rpm_obsoleted();
    CPPUNIT_ASSERT_EQUAL(1lu, install_set.size());
    CPPUNIT_ASSERT_EQUAL(install_set[0].get_full_nevra(), std::string("cmdline-0:1.2-3.noarch"));
    CPPUNIT_ASSERT_EQUAL(install_set[0].get_repo()->get_id(), std::string("@commandline"));
    CPPUNIT_ASSERT_EQUAL(0lu, reinstall_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, upgrade_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, downgrade_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, remove_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, obsoleted_set.size());
}

void BaseGoalTest::test_remove() {
    std::filesystem::path rpm_path =
        PROJECT_BINARY_DIR "/test/data/cmdline-rpms/noarch/cmdline-1.2-3.noarch.rpm";
    sack->add_system_package(rpm_path, false, false);
    libdnf::Goal goal(base.get());
    goal.add_rpm_remove("cmdline", {}, {});
    goal.resolve();
    auto install_set = goal.list_rpm_installs();
    auto reinstall_set = goal.list_rpm_reinstalls();
    auto upgrade_set = goal.list_rpm_upgrades();
    auto downgrade_set = goal.list_rpm_downgrades();
    auto remove_set = goal.list_rpm_removes();
    auto obsoleted_set = goal.list_rpm_obsoleted();
    CPPUNIT_ASSERT_EQUAL(0lu, install_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, reinstall_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, upgrade_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, downgrade_set.size());
    CPPUNIT_ASSERT_EQUAL(1lu, remove_set.size());
    CPPUNIT_ASSERT_EQUAL(remove_set[0].get_full_nevra(), std::string("cmdline-0:1.2-3.noarch"));
    CPPUNIT_ASSERT_EQUAL(0lu, obsoleted_set.size());
}

void BaseGoalTest::test_install_pkg() {
    std::filesystem::path rpm_path =
        PROJECT_SOURCE_DIR "/test/libdnf/rpm/repos-data/dnf-ci-fedora/x86_64/wget-1.19.5-5.fc29.x86_64.rpm";
    sack->add_system_package(rpm_path, false, false);
    libdnf::Goal goal(base.get());
    libdnf::rpm::SolvQuery query(&(base->get_rpm_solv_sack()));
    query.ifilter_available().ifilter_nevra(libdnf::sack::QueryCmp::EQ, {"wget-1.19.5-5.fc29.x86_64"});
    CPPUNIT_ASSERT_EQUAL(1lu, query.size());
    goal.add_rpm_install(query, true);
    goal.resolve();
    auto install_set = goal.list_rpm_installs();
    auto reinstall_set = goal.list_rpm_reinstalls();
    auto upgrade_set = goal.list_rpm_upgrades();
    auto downgrade_set = goal.list_rpm_downgrades();
    auto remove_set = goal.list_rpm_removes();
    auto obsoleted_set = goal.list_rpm_obsoleted();
    CPPUNIT_ASSERT_EQUAL(0lu, install_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, reinstall_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, upgrade_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, downgrade_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, remove_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, obsoleted_set.size());
}

void BaseGoalTest::test_install_or_reinstall() {
    std::filesystem::path rpm_path =
        PROJECT_SOURCE_DIR "/test/libdnf/rpm/repos-data/dnf-ci-fedora/x86_64/wget-1.19.5-5.fc29.x86_64.rpm";
    sack->add_system_package(rpm_path, false, false);
    libdnf::Goal goal(base.get());
    libdnf::rpm::SolvQuery query(&(base->get_rpm_solv_sack()));
    query.ifilter_available().ifilter_nevra(libdnf::sack::QueryCmp::EQ, {"wget-1.19.5-5.fc29.x86_64"});
    CPPUNIT_ASSERT_EQUAL(1lu, query.size());
    goal.add_rpm_install_or_reinstall(query, true);
    goal.resolve();
    auto install_set = goal.list_rpm_installs();
    auto reinstall_set = goal.list_rpm_reinstalls();
    auto upgrade_set = goal.list_rpm_upgrades();
    auto downgrade_set = goal.list_rpm_downgrades();
    auto remove_set = goal.list_rpm_removes();
    auto obsoleted_set = goal.list_rpm_obsoleted();
    CPPUNIT_ASSERT_EQUAL(0lu, install_set.size());
    CPPUNIT_ASSERT_EQUAL(1lu, reinstall_set.size());
    CPPUNIT_ASSERT_EQUAL(reinstall_set[0].get_full_nevra(), std::string("wget-0:1.19.5-5.fc29.x86_64"));
    CPPUNIT_ASSERT_EQUAL(0lu, upgrade_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, downgrade_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, remove_set.size());
    CPPUNIT_ASSERT_EQUAL(0lu, obsoleted_set.size());
}
