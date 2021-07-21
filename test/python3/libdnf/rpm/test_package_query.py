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

import support


class TestPackageQuery(support.LibdnfTestCase):
    def setUp(self):
        super().setUp()
        self.add_repo_repomd("repomd-repo1")

    def test_size(self):
        query = libdnf.rpm.PackageQuery(self.base)
        self.assertEqual(query.size(), 3)

    def test_iterate_package_query(self):
        query = libdnf.rpm.PackageQuery(self.base)

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

    def test_iterate_package_query2(self):
        # Tests the iteration of an unreferenced PackageQuery object. The iterator must hold a reference
        # to the iterated object, otherwise the gargabe collector can remove the object.

        # Iterates directly over "libdnf.rpm.PackageQuery(self.base)" result. No helping reference.
        prev_id = 0
        for pkg in libdnf.rpm.PackageQuery(self.base):
            id = pkg.get_id().id
            self.assertGreater(id, prev_id)
            prev_id = id
        self.assertLess(prev_id, self.sack.get_nsolvables())
        self.assertGreaterEqual(prev_id, libdnf.rpm.PackageQuery(self.base).size())

        # Another test. The iterator is created from the "query" reference, but the reference
        # is removed (set to "None") before starting the iteration.
        query = libdnf.rpm.PackageQuery(self.base)
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
        self.assertGreaterEqual(prev_id, libdnf.rpm.PackageQuery(self.base).size())

    def test_filter_name(self):
        # Test QueryCmp::EQ
        query = libdnf.rpm.PackageQuery(self.base)
        query.filter_name(["pkg"])
        self.assertEqual(query.size(), 1)
        # TODO(dmach): implement __str__()
        self.assertEqual([i.get_nevra() for i in query], ["pkg-1.2-3.x86_64"])

        # ---

        # Test QueryCmp::GLOB
        query = libdnf.rpm.PackageQuery(self.base)
        query.filter_name(["pk*"], libdnf.common.QueryCmp_GLOB)
        self.assertEqual(query.size(), 2)
        # TODO(dmach): implement __str__()
        self.assertEqual([i.get_nevra() for i in query], ["pkg-1.2-3.x86_64", "pkg-libs-1:1.3-4.x86_64"])
