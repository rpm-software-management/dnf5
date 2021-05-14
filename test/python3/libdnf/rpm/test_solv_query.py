# Copyright (C) 2020-2021 Red Hat, Inc.
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
import tempfile
import shutil
import os

import libdnf


class TestSolvQuery(unittest.TestCase):
    def setUp(self):
        self.base = libdnf.base.Base()

        # Sets path to cache directory.
        self.tmpdir = tempfile.mkdtemp(prefix="libdnf-python3-")
        self.base.get_config().cachedir().set(libdnf.conf.Option.Priority_RUNTIME, self.tmpdir)

        self.repo_sack = libdnf.rpm.RepoSack(self.base)
        self.sack = self.base.get_rpm_solv_sack()

        # Creates new repositories in the repo_sack
        repo = self.repo_sack.new_repo("repomd-repo1")

        # Tunes repository configuration (baseurl is mandatory)
        repo_path = os.path.join(os.getcwd(), "../../../test/data/repos-repomd/repomd-repo1/")
        baseurl = "file://" + repo_path
        repo_cfg = repo.get_config()
        repo_cfg.baseurl().set(libdnf.conf.Option.Priority_RUNTIME, baseurl)

        # Loads repository into rpm::Repo.
        repo.load()

        # Loads rpm::Repo into rpm::SolvSack
        self.sack.load_repo(repo.get(), libdnf.rpm.SolvSack.LoadRepoFlags_NONE)

    def tearDown(self):
        # Remove the cache directory.
        shutil.rmtree(self.tmpdir)

    def test_size(self):
        query = libdnf.rpm.SolvQuery(self.sack)
        self.assertEqual(query.size(), 3)

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
        # Test QueryCmp::EQ
        query = libdnf.rpm.SolvQuery(self.sack)
        query.ifilter_name(["pkg"])
        self.assertEqual(query.size(), 1)
        # TODO(dmach): implement __str__()
        self.assertEqual([i.get_nevra() for i in query], ["pkg-1.2-3.x86_64"])

        # ---

        # Test QueryCmp::GLOB
        query = libdnf.rpm.SolvQuery(self.sack)
        query.ifilter_name(["pk*"], libdnf.common.QueryCmp_GLOB)
        self.assertEqual(query.size(), 2)
        # TODO(dmach): implement __str__()
        self.assertEqual([i.get_nevra() for i in query], ["pkg-1.2-3.x86_64", "pkg-libs-1:1.3-4.x86_64"])
