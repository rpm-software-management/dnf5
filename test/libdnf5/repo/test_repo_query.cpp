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

#include "test_repo_query.hpp"

#include <libdnf5/base/base.hpp>
#include <libdnf5/repo/repo_sack.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION(RepoQueryTest);


void RepoQueryTest::test_query_basics() {
    auto repo_sack = base.get_repo_sack();

    // Creates new repositories in the repo_sack
    auto repo1 = repo_sack->create_repo("repo1");
    auto repo2 = repo_sack->create_repo("repo2");
    auto repo1_updates = repo_sack->create_repo("repo1_updates");
    auto repo2_updates = repo_sack->create_repo("repo2_updates");

    // Tunes configuration of repositories
    repo1->enable();
    repo2->disable();
    repo1_updates->disable();
    repo2_updates->enable();
    repo1->get_config().get_baseurl_option().set("file:///path/to/repo1");
    repo2->get_config().get_baseurl_option().set("https://host/path/to/repo2");
    repo1_updates->get_config().get_baseurl_option().set("https://host/path/to/repo1_updates");
    repo2_updates->get_config().get_baseurl_option().set("https://host/path/to/repo2_updates");

    // create a RepoQuery and test that it contains expected repos
    libdnf5::repo::RepoQuery repo_query(base);
    CPPUNIT_ASSERT_EQUAL(repo_query.size(), static_cast<size_t>(4));
    CPPUNIT_ASSERT((repo_query == libdnf5::Set{repo1, repo2, repo1_updates, repo2_updates}));

    // Tests filter_enabled method
    repo_query.filter_enabled(true);
    CPPUNIT_ASSERT((repo_query == libdnf5::Set{repo1, repo2_updates}));

    // Tests filter_id method
    libdnf5::repo::RepoQuery repo_query1(base);
    repo_query1.filter_id("*updates", libdnf5::sack::QueryCmp::GLOB);
    CPPUNIT_ASSERT((repo_query1 == libdnf5::Set{repo1_updates, repo2_updates}));

    // Tests filter_local method
    libdnf5::repo::RepoQuery repo_query2(base);
    repo_query2.filter_local(false);
    CPPUNIT_ASSERT((repo_query2 == libdnf5::Set{repo2, repo1_updates, repo2_updates}));

    // Tests iteration over RepoQuery object
    libdnf5::repo::RepoQuery repo_query3(base);
    libdnf5::Set<libdnf5::repo::RepoWeakPtr> result;
    for (auto repo : repo_query3) {
        result.add(repo);
    }
    CPPUNIT_ASSERT((result == libdnf5::Set{repo1, repo2, repo1_updates, repo2_updates}));
}
