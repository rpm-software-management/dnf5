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

import libdnf5

import base_test_case


class TestPackageQuery(base_test_case.BaseTestCase):
    def setUp(self):
        super().setUp()
        self.add_repo_repomd("repomd-repo1")

    def test_size(self):
        query = libdnf5.rpm.PackageQuery(self.base)
        self.assertEqual(query.size(), 3)

    def test_iterate_package_query(self):
        query = libdnf5.rpm.PackageQuery(self.base)

        # Iterates over reference "query".
        prev_id = 0
        for pkg in query:
            id = pkg.get_id().id
            self.assertGreater(id, prev_id)
            prev_id = id
        self.assertLess(prev_id, self.package_sack.get_nsolvables())
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
        self.assertLess(prev_id, self.package_sack.get_nsolvables())
        self.assertGreaterEqual(prev_id, query.size())

    def test_iterate_package_query2(self):
        # Tests the iteration of an unreferenced PackageQuery object. The iterator must hold a reference
        # to the iterated object, otherwise the gargabe collector can remove the object.

        # Iterates directly over "libdnf5.rpm.PackageQuery(self.base)" result. No helping reference.
        prev_id = 0
        for pkg in libdnf5.rpm.PackageQuery(self.base):
            id = pkg.get_id().id
            self.assertGreater(id, prev_id)
            prev_id = id
        self.assertLess(prev_id, self.package_sack.get_nsolvables())
        self.assertGreaterEqual(
            prev_id, libdnf5.rpm.PackageQuery(self.base).size())

        # Another test. The iterator is created from the "query" reference, but the reference
        # is removed (set to "None") before starting the iteration.
        query = libdnf5.rpm.PackageQuery(self.base)
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
        self.assertLess(prev_id, self.package_sack.get_nsolvables())
        self.assertGreaterEqual(
            prev_id, libdnf5.rpm.PackageQuery(self.base).size())

    def test_filter_name(self):
        # Test QueryCmp::EQ
        query = libdnf5.rpm.PackageQuery(self.base)
        query.filter_name(["pkg"])
        self.assertEqual(query.size(), 1)
        # TODO(dmach): implement __str__()
        self.assertEqual([i.get_nevra() for i in query], ["pkg-1.2-3.x86_64"])

        # ---

        # Test QueryCmp::GLOB
        query = libdnf5.rpm.PackageQuery(self.base)
        query.filter_name(["pk*"], libdnf5.common.QueryCmp_GLOB)
        self.assertEqual(query.size(), 2)
        # TODO(dmach): implement __str__()
        self.assertEqual([i.get_nevra() for i in query], [
                         "pkg-1.2-3.x86_64", "pkg-libs-1:1.3-4.x86_64"])

    def test_resolve_pkg_spec(self):
        # Test passing empty forms
        query = libdnf5.rpm.PackageQuery(self.base)
        settings = libdnf5.base.ResolveSpecSettings()

        match, nevra = query.resolve_pkg_spec("pkg*", settings, True)

        self.assertEqual(query.size(), 2)
        self.assertEqual([i.get_nevra() for i in query], [
                         "pkg-1.2-3.x86_64", "pkg-libs-1:1.3-4.x86_64"])
        self.assertTrue(match)
        self.assertEqual(nevra.get_name(), "pkg*")
        self.assertTrue(nevra.has_just_name())

        # Test passing an explicit list of forms
        query = libdnf5.rpm.PackageQuery(self.base)
        settings = libdnf5.base.ResolveSpecSettings()
        settings.set_nevra_forms(
            libdnf5.rpm.VectorNevraForm(1, libdnf5.rpm.Nevra.Form_NA))

        match, nevra = query.resolve_pkg_spec("pkg.x86_64", settings, True)

        self.assertEqual(query.size(), 1)
        self.assertEqual([i.get_nevra() for i in query], ["pkg-1.2-3.x86_64"])
        self.assertTrue(match)
        self.assertEqual(nevra.get_name(), "pkg")
        self.assertEqual(nevra.get_arch(), "x86_64")
        self.assertFalse(nevra.has_just_name())

    def test_pkg_reason_value(self):
        # Test wrapper for reason enum
        query = libdnf5.rpm.PackageQuery(self.base)
        query.filter_name(["pkg"])
        package = next(iter(query))
        self.assertEqual(package.get_reason(),
                         libdnf5.transaction.TransactionItemReason_NONE)

    def test_pkg_query_without_setup(self):
        # Create a new Base object
        base = libdnf5.base.Base()

        # Try to create a package query without running base.setup()
        self.assertRaises(RuntimeError, libdnf5.rpm.PackageQuery, base)

    def test_pkg_get_changelogs(self):
        # Test wrapper for changelogs vector
        query = libdnf5.rpm.PackageQuery(self.base)
        query.filter_name(["pkg"])
        package = next(iter(query))
        logs = package.get_changelogs()
        self.assertEqual(2, logs.size())
        log = next(iter(logs))
        self.assertEqual('First change', log.text)
        self.assertEqual('Joe Black', log.author)
        self.assertEqual(1641027600, log.timestamp)
