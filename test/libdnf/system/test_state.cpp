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

#include <iostream>


CPPUNIT_TEST_SUITE_REGISTRATION(StateTest);

using namespace libdnf;

void StateTest::setUp() {
    BaseTestCase::setUp();
    add_repo_repomd("repomd-repo1");

    temp_dir = std::make_unique<libdnf::utils::fs::TempDir>("libdnf_test_state");

    std::ofstream toml(temp_dir->get_path() / "userinstalled.toml");
    toml << "userinstalled = [\n"
            "\"pkg.x86_64\",\n"
            "\"cmdline.noarch\",\n"
            "]\n";
    toml.close();
}

void StateTest::tearDown() {
    temp_dir.reset();

    BaseTestCase::tearDown();
}

void StateTest::test_state_read() {
    libdnf::system::State state("/", temp_dir->get_path());

    CPPUNIT_ASSERT_EQUAL(libdnf::transaction::TransactionItemReason::USER, state.get_reason("pkg.x86_64"));
    CPPUNIT_ASSERT_EQUAL(libdnf::transaction::TransactionItemReason::DEPENDENCY, state.get_reason("pkg-libs.x86_64"));
}

void StateTest::test_state_write() {
    const auto path = temp_dir->get_path() / "write_test";
    libdnf::system::State state("/", path);

    state.set_reason("pkg.x86_64", libdnf::transaction::TransactionItemReason::USER);
    state.set_reason("pkg-libs.x86_64", libdnf::transaction::TransactionItemReason::USER);
    state.set_reason("unresolvable.noarch", libdnf::transaction::TransactionItemReason::USER);
    state.set_reason("unresolvable.noarch", libdnf::transaction::TransactionItemReason::DEPENDENCY);

    state.save();

    std::ifstream toml(path / "userinstalled.toml");
    std::string contents;
    contents.assign(std::istreambuf_iterator<char>(toml), std::istreambuf_iterator<char>());

    std::string expected =
        "userinstalled = [\n"
        "\"pkg-libs.x86_64\",\n"
        "\"pkg.x86_64\",\n"
        "]\n";
    CPPUNIT_ASSERT_EQUAL(expected, contents);
}
