# Copyright Contributors to the libdnf project.
#
# This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
#
# Libdnf is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Libdnf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

import unittest

import libdnf

class TestRepQueryo(unittest.TestCase):
    def test_repo_query(self):
        base = libdnf.base.Base()

        # Sets Base internals according to configuration
        base.setup()

        repo_sack = base.get_repo_sack()

        # Creates new repositories in the repo_sack
        repo1 = repo_sack.create_repo("repo1")
        repo2 = repo_sack.create_repo("repo2")
        repo1_updates = repo_sack.create_repo("repo1_updates")
        repo2_updates = repo_sack.create_repo("repo2_updates")

        # Tunes configuration of repositories
        repo1.enable();
        repo2.disable();
        repo1_updates.disable();
        repo2_updates.enable();
        repo1.get_config().baseurl().set(libdnf.conf.Option.Priority_RUNTIME, "file:///path/to/repo1")
        repo2.get_config().baseurl().set(libdnf.conf.Option.Priority_RUNTIME, "https://host/path/to/repo2")
        repo1_updates.get_config().baseurl().set(libdnf.conf.Option.Priority_RUNTIME, "https://host/path/to/repo1_updates")
        repo2_updates.get_config().baseurl().set(libdnf.conf.Option.Priority_RUNTIME, "https://host/path/to/repo2_updates")

        # create a RepoQuery and test that it contains expected repos
        repo_query = libdnf.repo.RepoQuery(base)
        self.assertEqual(repo_query.size(), 4)
        self.assertEqual(len(repo_query), 4)
        self.assertEqual(set(repo_query), {repo1, repo2, repo1_updates, repo2_updates})

        # Tests filter_enabled method
        repo_query.filter_enabled(True);
        self.assertEqual(set(repo_query), {repo1, repo2_updates})

        # Tests filter_id method
        repo_query1 = libdnf.repo.RepoQuery(base)
        repo_query1.filter_id("*updates", libdnf.common.QueryCmp_GLOB);
        self.assertEqual(set(repo_query1), {repo1_updates, repo2_updates})

        # Tests filter_local method
        repo_query2 = libdnf.repo.RepoQuery(base)
        repo_query2.filter_local(False);
        self.assertEqual(set(repo_query2), {repo2, repo1_updates, repo2_updates})

        # Tests iteration over RepoQuery object
        repo_query3 = libdnf.repo.RepoQuery(base)
        result = set()
        for repo in repo_query3:
            result.add(repo.get_id())
        self.assertEqual(result, {"repo1", "repo2", "repo1_updates", "repo2_updates"})
