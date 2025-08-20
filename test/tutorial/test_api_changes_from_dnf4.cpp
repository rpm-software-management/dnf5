/*
 * Copyright Contributors to the libdnf project.
 *
 * This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
 *
 * Libdnf is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Libdnf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "test_api_changes_from_dnf4.hpp"

#include <libdnf5/base/base.hpp>
#include <libdnf5/base/goal.hpp>
#include <libdnf5/comps/group/query.hpp>
#include <libdnf5/rpm/package_query.hpp>


CPPUNIT_TEST_SUITE_REGISTRATION(APIChangesTest);


void APIChangesTest::setUp() {
    temp = std::make_unique<libdnf5::utils::fs::TempDir>("libdnf_unittest");
    installroot = temp->get_path() / "installroot";
    cachedir = installroot / "var/cache/dnf/";
}


void APIChangesTest::tearDown() {}


void APIChangesTest::test_create_base() {
#include "api_changes_from_dnf4/create_base.cpp"
}


void APIChangesTest::test_configure_base() {
#include "api_changes_from_dnf4/create_base.cpp"
// Comment necessary to have this order of includes.
#include "api_changes_from_dnf4/configure_base.cpp"
}


void APIChangesTest::test_load_repos() {
#include "api_changes_from_dnf4/create_base.cpp"
#include "api_changes_from_dnf4/load_repos.cpp"
}


void APIChangesTest::test_package_query() {
#include "api_changes_from_dnf4/create_base.cpp"
// Comment necessary to have this order of includes.
#include "api_changes_from_dnf4/load_repos.cpp"
#include "api_changes_from_dnf4/package_query.cpp"
}


void APIChangesTest::test_group_query() {
#include "api_changes_from_dnf4/create_base.cpp"
// Comment necessary to have this order of includes.
#include "api_changes_from_dnf4/load_repos.cpp"
// Comment necessary to have this order of includes.
#include "api_changes_from_dnf4/group_query.cpp"
}


void APIChangesTest::test_transaction() {
#include "api_changes_from_dnf4/create_base.cpp"
// Comment necessary to have this order of includes.
#include "api_changes_from_dnf4/load_repos.cpp"
#include "api_changes_from_dnf4/transaction.cpp"
}
