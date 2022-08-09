/*
Copyright (C) 2021 Red Hat, Inc.

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


#include "test_state.hpp"

#include "utils.hpp"
#include "utils/fs/file.hpp"


CPPUNIT_TEST_SUITE_REGISTRATION(StateTest);

using namespace libdnf;

static const std::string packages_contents{R"""(version = "1.0"
[packages]
"pkg.x86_64" = {reason="User"}
"unresolvable.noarch" = {reason="Dependency"}
"pkg-libs.x86_64" = {reason="Dependency"}
)"""};

static const std::string nevras_contents{R"""(version = "1.0"
[nevras]
"unresolvable-1.2-1.noarch" = {from_repo="repo2"}
"pkg-libs-1.2-1.x86_64" = {from_repo=""}
"pkg-1.2-1.x86_64" = {from_repo="repo1"}
)"""};

// TODO(lukash) alphabetic sorting
static const std::string groups_contents{R"""(version = "1.0"
[groups]
group-2 = {packages=["pkg1","pkg2"],userinstalled=false}
group-1 = {packages=["foo","bar"],userinstalled=true}
)"""};

static const std::string modules_contents{R"""(version = "1.0"
[modules]
module-2 = {installed_profiles=[],state="Disabled",enabled_stream="stream-2"}
[modules.module-1]
installed_profiles = ["zigg","zagg"]
state = "Enabled"
enabled_stream = "stream-1"
)"""};

// workaround, because of different behavior before toml 3.7.1
static std::string trim(std::string str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char c) { return c != '\n'; }));
    str.erase(
        std::unique(str.begin(), str.end(), [](char c1, char c2) { return c1 == '\n' && c2 == '\n'; }), str.end());

    return str;
}

void StateTest::setUp() {
    BaseTestCase::setUp();
    add_repo_repomd("repomd-repo1");

    temp_dir = std::make_unique<libdnf::utils::fs::TempDir>("libdnf_test_state");

    libdnf::utils::fs::File(temp_dir->get_path() / "packages.toml", "w").write(packages_contents);
    libdnf::utils::fs::File(temp_dir->get_path() / "nevras.toml", "w").write(nevras_contents);
    libdnf::utils::fs::File(temp_dir->get_path() / "groups.toml", "w").write(groups_contents);
    libdnf::utils::fs::File(temp_dir->get_path() / "modules.toml", "w").write(modules_contents);
}

void StateTest::tearDown() {
    temp_dir.reset();

    BaseTestCase::tearDown();
}

void StateTest::test_state_version() {
    libdnf::utils::fs::File(temp_dir->get_path() / "packages.toml", "w").write(R"""(version = "aaa"
[packages])""");

    CPPUNIT_ASSERT_THROW(libdnf::system::State(temp_dir->get_path()), libdnf::system::InvalidVersionError);

    libdnf::utils::fs::File(temp_dir->get_path() / "packages.toml", "w").write(R"""(version = "4.0"
[packages])""");

    CPPUNIT_ASSERT_THROW(libdnf::system::State(temp_dir->get_path()), libdnf::system::UnsupportedVersionError);
}

void StateTest::test_state_read() {
    libdnf::system::State state(temp_dir->get_path());

    CPPUNIT_ASSERT_EQUAL(transaction::TransactionItemReason::USER, state.get_package_reason("pkg.x86_64"));
    CPPUNIT_ASSERT_EQUAL(transaction::TransactionItemReason::DEPENDENCY, state.get_package_reason("pkg-libs.x86_64"));

    CPPUNIT_ASSERT_EQUAL(std::string("repo1"), state.get_package_from_repo("pkg-1.2-1.x86_64"));
    CPPUNIT_ASSERT_EQUAL(std::string("repo2"), state.get_package_from_repo("unresolvable-1.2-1.noarch"));

    libdnf::system::GroupState grp_state_1{.userinstalled = true, .packages = {"foo", "bar"}};
    CPPUNIT_ASSERT_EQUAL(grp_state_1, state.get_group_state("group-1"));
    libdnf::system::GroupState grp_state_2{.userinstalled = false, .packages = {"pkg1", "pkg2"}};
    CPPUNIT_ASSERT_EQUAL(grp_state_2, state.get_group_state("group-2"));

    CPPUNIT_ASSERT_EQUAL(std::string("stream-1"), state.get_module_enabled_stream("module-1"));
    CPPUNIT_ASSERT_EQUAL(libdnf::module::ModuleState::ENABLED, state.get_module_state("module-1"));
    std::vector<std::string> module_1_installed_profiles{"zigg", "zagg"};
    CPPUNIT_ASSERT_EQUAL(module_1_installed_profiles, state.get_module_installed_profiles("module-1"));
    CPPUNIT_ASSERT_EQUAL(std::string("stream-2"), state.get_module_enabled_stream("module-2"));
    CPPUNIT_ASSERT_EQUAL(libdnf::module::ModuleState::DISABLED, state.get_module_state("module-2"));
}

void StateTest::test_state_write() {
    const auto path = temp_dir->get_path() / "write_test";
    libdnf::system::State state(path);

    state.set_package_reason("pkg.x86_64", transaction::TransactionItemReason::USER);
    state.set_package_reason("pkg-libs.x86_64", transaction::TransactionItemReason::DEPENDENCY);
    state.set_package_reason("unresolvable.noarch", transaction::TransactionItemReason::USER);
    state.set_package_reason("unresolvable.noarch", transaction::TransactionItemReason::DEPENDENCY);

    state.set_package_from_repo("pkg-1.2-1.x86_64", "repo1");
    state.set_package_from_repo("unresolvable-1.2-1.noarch", "repo2");
    state.set_package_from_repo("pkg-libs-1.2-1.x86_64", "");

    state.set_group_state("group-1", {.userinstalled = true, .packages = {"foo", "bar"}});
    state.set_group_state("group-2", {.userinstalled = false, .packages = {"pkg1", "pkg2"}});

    state.set_module_enabled_stream("module-1", "stream-1");
    state.set_module_state("module-1", libdnf::module::ModuleState::ENABLED);
    state.set_module_installed_profiles("module-1", {"zigg", "zagg"});

    state.set_module_enabled_stream("module-2", "stream-2");
    state.set_module_state("module-2", libdnf::module::ModuleState::DISABLED);

    state.save();

    CPPUNIT_ASSERT_EQUAL(packages_contents, trim(libdnf::utils::fs::File(path / "packages.toml", "r").read()));
    CPPUNIT_ASSERT_EQUAL(nevras_contents, trim(libdnf::utils::fs::File(path / "nevras.toml", "r").read()));
    CPPUNIT_ASSERT_EQUAL(groups_contents, trim(libdnf::utils::fs::File(path / "groups.toml", "r").read()));
    CPPUNIT_ASSERT_EQUAL(modules_contents, trim(libdnf::utils::fs::File(path / "modules.toml", "r").read()));

    // Test removes
    state.remove_package_na_state("pkg.x86_64");
    state.remove_package_nevra_state("pkg-1.2-1.x86_64");
    state.remove_group_state("group-1");
    state.remove_module_state("module-1");

    state.save();

    const std::string packages_contents_after_remove{R"""(version = "1.0"
[packages]
"unresolvable.noarch" = {reason="Dependency"}
"pkg-libs.x86_64" = {reason="Dependency"}
)"""};

    CPPUNIT_ASSERT_EQUAL(
        packages_contents_after_remove, trim(libdnf::utils::fs::File(path / "packages.toml", "r").read()));

    const std::string nevras_contents_after_remove{R"""(version = "1.0"
[nevras]
"unresolvable-1.2-1.noarch" = {from_repo="repo2"}
"pkg-libs-1.2-1.x86_64" = {from_repo=""}
)"""};

    CPPUNIT_ASSERT_EQUAL(nevras_contents_after_remove, trim(libdnf::utils::fs::File(path / "nevras.toml", "r").read()));

    const std::string groups_contents_after_remove{
        R"""(version = "1.0"
groups = {group-2={packages=["pkg1","pkg2"],userinstalled=false}}
)"""};

    CPPUNIT_ASSERT_EQUAL(groups_contents_after_remove, trim(libdnf::utils::fs::File(path / "groups.toml", "r").read()));

    const std::string modules_contents_after_remove{R"""(version = "1.0"
[modules]
module-2 = {installed_profiles=[],state="Disabled",enabled_stream="stream-2"}
)"""};

    CPPUNIT_ASSERT_EQUAL(
        modules_contents_after_remove, trim(libdnf::utils::fs::File(path / "modules.toml", "r").read()));
}
