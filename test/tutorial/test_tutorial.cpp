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


#include "test_tutorial.hpp"

#include <libdnf5/base/base.hpp>
#include <libdnf5/base/goal.hpp>
#include <libdnf5/repo/package_downloader.hpp>
#include <libdnf5/rpm/package_query.hpp>


CPPUNIT_TEST_SUITE_REGISTRATION(TutorialTest);


void TutorialTest::setUp() {
    temp = std::make_unique<libdnf5::utils::fs::TempDir>("libdnf_unittest");
    installroot = temp->get_path() / "installroot";
    cachedir = installroot / "var/cache/dnf/";
}


void TutorialTest::tearDown() {}


void TutorialTest::test_create_base() {
#include "session/create_base.cpp"
}


void TutorialTest::test_load_repo() {
#include "session/create_base.cpp"

    base.get_config().get_cachedir_option().set(cachedir);

#include "repo/load_repo.cpp"
}


void TutorialTest::test_load_system_repos() {
#include "session/create_base.cpp"

    base.get_config().get_cachedir_option().set(cachedir);

#include "repo/load_system_repos.cpp"
}


void TutorialTest::test_query() {
#include "session/create_base.cpp"

    base.get_config().get_cachedir_option().set(cachedir);

#include "query/query.cpp"
#include "repo/load_repo.cpp"
}


void TutorialTest::test_transaction() {
#include "session/create_base.cpp"

    base.get_config().get_cachedir_option().set(cachedir);

#include "repo/load_repo.cpp"
#include "transaction/transaction.cpp"
}


void TutorialTest::test_force_arch() {
#include "session/force_arch.cpp"
}
