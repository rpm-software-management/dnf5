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

#include "test_repo_query.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/repo/repo_sack.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(RepoQueryTest);


void RepoQueryTest::setUp() {}


void RepoQueryTest::tearDown() {}


void RepoQueryTest::test_query_basics() {
    libdnf::Base base;
    auto repo_sack = base.get_repo_sack();
    //libdnf::repo::RepoSack repo_sack(base);
    //auto repo_sack_weak_ptr = repo_sack.get_weak_ptr();

    // TODO(lukash) repo initialization requires to have the cachedir set; do it differently?
    base.get_config().cachedir().set(libdnf::Option::Priority::RUNTIME, base.get_config().system_cachedir().get_value());

    // Creates new repositories in the repo_sack
    auto repo1 = repo_sack->new_repo("repo1");
    auto repo2 = repo_sack->new_repo("repo2");
    auto repo1_updates = repo_sack->new_repo("repo1_updates");
    auto repo2_updates = repo_sack->new_repo("repo2_updates");

    // Tunes configuration of repositories
    repo1->enable();
    repo2->disable();
    repo1_updates->disable();
    repo2_updates->enable();
    repo1->get_config().baseurl().set(libdnf::Option::Priority::RUNTIME, "file:///path/to/repo1");
    repo2->get_config().baseurl().set(libdnf::Option::Priority::RUNTIME, "https://host/path/to/repo2");
    repo1_updates->get_config().baseurl().set(libdnf::Option::Priority::RUNTIME, "https://host/path/to/repo1_updates");
    repo2_updates->get_config().baseurl().set(libdnf::Option::Priority::RUNTIME, "https://host/path/to/repo2_updates");

    // create a RepoQuery and test that it contains expected repos
    libdnf::repo::RepoQuery repo_query(base);
    CPPUNIT_ASSERT_EQUAL(repo_query.size(), static_cast<size_t>(4));
    CPPUNIT_ASSERT((repo_query == libdnf::Set{repo1, repo2, repo1_updates, repo2_updates}));

    // Tests filter_enabled method
    repo_query.filter_enabled(true);
    CPPUNIT_ASSERT((repo_query == libdnf::Set{repo1, repo2_updates}));

    // Tests filter_id method
    libdnf::repo::RepoQuery repo_query1(base);
    repo_query1.filter_id("*updates", libdnf::sack::QueryCmp::GLOB);
    CPPUNIT_ASSERT((repo_query1 == libdnf::Set{repo1_updates, repo2_updates}));

    // Tests filter_local method
    libdnf::repo::RepoQuery repo_query2(base);
    repo_query2.filter_local(false);
    CPPUNIT_ASSERT((repo_query2 == libdnf::Set{repo2, repo1_updates, repo2_updates}));

    // Tests iteration over RepoQuery object
    libdnf::repo::RepoQuery repo_query3(base);
    libdnf::Set<libdnf::repo::RepoWeakPtr> result;
    for (auto repo : repo_query3) {
        result.add(repo);
    }
    CPPUNIT_ASSERT((result == libdnf::Set{repo1, repo2, repo1_updates, repo2_updates}));
}
