# Copyright (C) 2020 Red Hat, Inc.
#
# This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
#
# Libdnf is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Libdnf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

import unittest
import os

import libdnf


class TestSolvQuery(unittest.TestCase):
    def setUp(self):
        self.base = libdnf.base.Base()

        # Sets path to cache directory.
        cwd = os.getcwd()
        self.base.get_config().cachedir().set(libdnf.conf.Option.Priority_RUNTIME, cwd)

        self.repo_sack = libdnf.rpm.RepoSack(self.base)
        self.sack = libdnf.rpm.SolvSack(self.base)

        # Creates new repositories in the repo_sack
        repo = self.repo_sack.new_repo("dnf-ci-fedora")

        # Tunes repositotory configuration (baseurl is mandatory)
        repo_path = os.path.join(cwd, "../../../test/libdnf/rpm/repos-data/dnf-ci-fedora/")
        baseurl = "file://" + repo_path
        repo_cfg = repo.get_config()
        repo_cfg.baseurl().set(libdnf.conf.Option.Priority_RUNTIME, baseurl)

        # Loads repository into rpm::Repo.
        repo.load()

        # Loads rpm::Repo into rpm::SolvSack
        self.sack.load_repo(repo.get(), libdnf.rpm.SolvSack.LoadRepoFlags_NONE)

    def test_size(self):
        query = libdnf.rpm.SolvQuery(self.sack)
        self.assertEqual(query.size(), 291)

    def test_iterate_solv_query(self):
        query = libdnf.rpm.SolvQuery(self.sack)

        # Iterates over reference "query".
        prev_id = 0
        for pkg in query:
            id = pkg.get_id().id
            self.assertGreater(id, prev_id)
            prev_id = id
        self.assertLess(prev_id, self.sack.get_nsolvables())
        self.assertGreaterEqual(prev_id, query.size())

        # Similar to above, but longer notation.
        query_iterator = iter(query)
        prev_id = 0
        while True:
            try:
                pkg = next(query_iterator)
                id = pkg.get_id().id
                self.assertGreater(id, prev_id)
                prev_id = id
            except StopIteration:
                break
        self.assertLess(prev_id, self.sack.get_nsolvables())
        self.assertGreaterEqual(prev_id, query.size())

    def test_iterate_solv_query2(self):
        # Tests the iteration of an unreferenced SolvQuery object. The iterator must hold a reference
        # to the iterated object, otherwise the gargabe collector can remove the object.

        # Iterates directly over "libdnf.rpm.SolvQuery(self.sack)" result. No helping reference.
        prev_id = 0
        for pkg in libdnf.rpm.SolvQuery(self.sack):
            id = pkg.get_id().id
            self.assertGreater(id, prev_id)
            prev_id = id
        self.assertLess(prev_id, self.sack.get_nsolvables())
        self.assertGreaterEqual(prev_id, libdnf.rpm.SolvQuery(self.sack).size())

        # Another test. The iterator is created from the "query" reference, but the reference
        # is removed (set to "None") before starting the iteration.
        query = libdnf.rpm.SolvQuery(self.sack)
        query_iterator = iter(query)
        query = None
        prev_id = 0
        while True:
            try:
                pkg = next(query_iterator)
                id = pkg.get_id().id
                self.assertGreater(id, prev_id)
                prev_id = id
            except StopIteration:
                break
        self.assertLess(prev_id, self.sack.get_nsolvables())
        self.assertGreaterEqual(prev_id, libdnf.rpm.SolvQuery(self.sack).size())

    def test_ifilter_name(self):

        nevras = {"CQRlib-1.1.1-4.fc29.src", "CQRlib-1.1.1-4.fc29.x86_64"}
        nevras_contains = {"CQRlib-1.1.1-4.fc29.src", "CQRlib-1.1.1-4.fc29.x86_64",
                           "CQRlib-devel-1.1.2-16.fc29.src", "CQRlib-devel-1.1.2-16.fc29.x86_64"}
        full_nevras = {"CQRlib-0:1.1.1-4.fc29.src", "CQRlib-0:1.1.1-4.fc29.x86_64",
                       "nodejs-1:5.12.1-1.fc29.src", "nodejs-1:5.12.1-1.fc29.x86_64"}

        # Test QueryCmp::EQ
        query = libdnf.rpm.SolvQuery(self.sack)
        names = ["CQRlib"]
        query.ifilter_name(libdnf.common.QueryCmp_EQ, names)
        self.assertEqual(query.size(), 2)
        for pkg in query:
            self.assertTrue(pkg.get_nevra() in nevras)

        # Test QueryCmp::GLOB
        query2 = libdnf.rpm.SolvQuery(self.sack)
        names2 = ["CQ?lib"]
        query2.ifilter_name(libdnf.common.QueryCmp_GLOB, names2)
        self.assertEqual(query2.size(), 2)
        for pkg in query2:
            self.assertTrue(pkg.get_nevra() in nevras)
