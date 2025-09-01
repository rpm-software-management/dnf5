# Copyright (C) 2021 Red Hat, Inc.
# SPDX-License-Identifier: LGPL-2.0-or-later
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


import libdnf5

import base_test_case


class TestPackageQuerySetOperators(base_test_case.BaseTestCase):
    def setUp(self):
        super().setUp()
        self.add_repo_solv("solv-24pkgs")

    def test_update(self):
        # packages with releases: 1, 2
        q1 = libdnf5.rpm.PackageQuery(self.base)
        q1.filter_release(["1", "2"])
        self.assertEqual(q1.size(), 2)

        # packages with releases: 2, 3
        q2 = libdnf5.rpm.PackageQuery(self.base)
        q2.filter_release(["2", "3"])
        self.assertEqual(q2.size(), 2)

        # union is all 3 packages
        q1.update(q2)
        self.assertEqual(q1.size(), 3)

    def test_difference(self):
        # packages with releases: 1, 2
        q1 = libdnf5.rpm.PackageQuery(self.base)
        q1.filter_release(["1", "2"])
        self.assertEqual(q1.size(), 2)

        q2 = libdnf5.rpm.PackageQuery(self.base)
        q2.filter_release(["2", "3"])
        self.assertEqual(q2.size(), 2)

        q1.difference(q2)
        self.assertEqual(q1.size(), 1)

        # difference is the package with release 1
        pkg = list(q1)[0]
        self.assertEqual(pkg.get_release(), "1")

    def test_intersection(self):
        # packages with releases: 1, 2
        q1 = libdnf5.rpm.PackageQuery(self.base)
        q1.filter_release(["1", "2"])
        self.assertEqual(q1.size(), 2)

        # packages with releases: 2, 3
        q2 = libdnf5.rpm.PackageQuery(self.base)
        q2.filter_release(["2", "3"])
        self.assertEqual(q2.size(), 2)

        # intersection is the package with release 2
        q1.intersection(q2)
        self.assertEqual(q1.size(), 1)

        pkg = list(q1)[0]
        self.assertEqual(pkg.get_release(), "2")
