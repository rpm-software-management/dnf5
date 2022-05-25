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

static const std::string packages_contents{R"""(
[packages]
"pkg.x86_64" = {reason="User"}
"unresolvable.noarch" = {reason="Dependency"}
"pkg-libs.x86_64" = {reason="Dependency"}

)"""};

static const std::string nevras_contents{R"""(
[nevras]
"unresolvable-1.2-1.noarch" = {from_repo="repo2"}
"pkg-libs-1.2-1.x86_64" = {from_repo=""}
"pkg-1.2-1.x86_64" = {from_repo="repo1"}

)"""};


void StateTest::setUp() {
    BaseTestCase::setUp();
    add_repo_repomd("repomd-repo1");

    temp_dir = std::make_unique<libdnf::utils::fs::TempDir>("libdnf_test_state");

    libdnf::utils::fs::File(temp_dir->get_path() / "packages.toml", "w").write(packages_contents);
    libdnf::utils::fs::File(temp_dir->get_path() / "nevras.toml", "w").write(nevras_contents);
}

void StateTest::tearDown() {
    temp_dir.reset();

    BaseTestCase::tearDown();
}

void StateTest::test_state_read() {
    libdnf::system::State state(temp_dir->get_path());

    CPPUNIT_ASSERT_EQUAL(transaction::TransactionItemReason::USER, state.get_package_reason("pkg.x86_64"));
    CPPUNIT_ASSERT_EQUAL(transaction::TransactionItemReason::DEPENDENCY, state.get_package_reason("pkg-libs.x86_64"));

    CPPUNIT_ASSERT_EQUAL(std::string("repo1"), state.get_package_from_repo("pkg-1.2-1.x86_64"));
    CPPUNIT_ASSERT_EQUAL(std::string("repo2"), state.get_package_from_repo("unresolvable-1.2-1.noarch"));
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

    state.save();

    CPPUNIT_ASSERT_EQUAL(packages_contents, libdnf::utils::fs::File(path / "packages.toml", "r").read());
    CPPUNIT_ASSERT_EQUAL(nevras_contents, libdnf::utils::fs::File(path / "nevras.toml", "r").read());

    // Test removes
    state.remove_package_na_state("pkg.x86_64");
    state.remove_package_nevra_state("pkg-1.2-1.x86_64");

    state.save();

    const std::string packages_contents_after_remove{R"""(
[packages]
"unresolvable.noarch" = {reason="Dependency"}
"pkg-libs.x86_64" = {reason="Dependency"}

)"""};
    CPPUNIT_ASSERT_EQUAL(packages_contents_after_remove, libdnf::utils::fs::File(path / "packages.toml", "r").read());

    const std::string nevras_contents_after_remove{R"""(
[nevras]
"unresolvable-1.2-1.noarch" = {from_repo="repo2"}
"pkg-libs-1.2-1.x86_64" = {from_repo=""}

)"""};
    CPPUNIT_ASSERT_EQUAL(nevras_contents_after_remove, libdnf::utils::fs::File(path / "nevras.toml", "r").read());
}
