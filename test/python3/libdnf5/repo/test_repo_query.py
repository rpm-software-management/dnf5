# Copyright Contributors to the DNF5 project.
# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later

import unittest

import libdnf5

import base_test_case


class TestRepoQuery(base_test_case.BaseTestCase):
    def test_repo_query(self):
        # Creates new repositories in the repo_sack
        repo1 = self.repo_sack.create_repo("repo1")
        repo1.enable()
        repo1.get_config().baseurl = "file:///path/to/repo1"

        repo2 = self.repo_sack.create_repo("repo2")
        repo2.disable()
        repo2.get_config().baseurl = "https://host/path/to/repo2"

        repo1_updates = self.repo_sack.create_repo("repo1_updates")
        repo1_updates.disable()
        repo1_updates.get_config().baseurl = "https://host/path/to/repo1_updates"

        repo2_updates = self.repo_sack.create_repo("repo2_updates")
        repo2_updates.enable()
        repo2_updates.get_config().baseurl = "https://host/path/to/repo2_updates"

        # create a RepoQuery and test that it contains expected repos
        repo_query = libdnf5.repo.RepoQuery(self.base)
        self.assertEqual(repo_query.size(), 4)
        self.assertEqual(len(repo_query), 4)
        self.assertEqual(set(repo_query), {
                         repo1, repo2, repo1_updates, repo2_updates})

        # Tests filter_enabled method
        repo_query.filter_enabled(True)
        self.assertEqual(set(repo_query), {repo1, repo2_updates})

        # Tests filter_id method
        repo_query1 = libdnf5.repo.RepoQuery(self.base)
        repo_query1.filter_id("*updates", libdnf5.common.QueryCmp_GLOB)
        self.assertEqual(set(repo_query1), {repo1_updates, repo2_updates})

        # Tests filter_local method
        repo_query2 = libdnf5.repo.RepoQuery(self.base)
        repo_query2.filter_local(False)
        self.assertEqual(set(repo_query2), {
                         repo2, repo1_updates, repo2_updates})

        # Tests iteration over RepoQuery object
        repo_query3 = libdnf5.repo.RepoQuery(self.base)
        result = set()
        for repo in repo_query3:
            result.add(repo.get_id())
        self.assertEqual(
            result, {"repo1", "repo2", "repo1_updates", "repo2_updates"})

        # Tests repo objects after query goes out of scope
        repos_list = []
        for repo in libdnf5.repo.RepoQuery(self.base):
            repos_list.append(repo)
        result = set()
        for repo in repos_list:
            result.add(repo.get_id())
        self.assertEqual(
            result, {"repo1", "repo2", "repo1_updates", "repo2_updates"})
