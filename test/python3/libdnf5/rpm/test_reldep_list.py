# Copyright Contributors to the DNF5 project.
# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later
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


class TestReldepList(base_test_case.BaseTestCase):
    def setUp(self):
        super().setUp()
        self.add_repo_repomd("repomd-repo1")

    def test_add(self):
        a = libdnf5.rpm.Reldep(self.base, "python3-labirinto = 4.2.0")
        b = libdnf5.rpm.Reldep(self.base, "(lab-list if labirinto.txt)")

        list1 = libdnf5.rpm.ReldepList(self.base)
        self.assertEqual(list1.size(), 0)
        list1.add(a)
        # TODO(jrohel): Shall we have add() with ReldepId as a part of public API?
        list1.add(b.get_id())
        list1.add_reldep("delgado > 1.2")
        self.assertEqual(list1.size(), 3)

    def test_get(self):
        a = libdnf5.rpm.Reldep(self.base, "python3-labirinto = 4.2.0")
        b = libdnf5.rpm.Reldep(self.base, "(lab-list if labirinto.txt)")
        c = libdnf5.rpm.Reldep(self.base, "(labirinto unless labirinto_c)")

        list1 = libdnf5.rpm.ReldepList(self.base)
        list1.add(a)
        list1.add(b)
        list1.add(c)

        self.assertEqual(list1.get(0), a)
        self.assertEqual(list1.get(1).to_string(),
                         "(lab-list if labirinto.txt)")
        self.assertEqual(list1.get_id(2).id, c.get_id().id)

    def test_compare(self):
        a = libdnf5.rpm.Reldep(self.base, "python3-labirinto = 4.2.0")
        b = libdnf5.rpm.Reldep(self.base, "(lab-list if labirinto.txt)")

        list1 = libdnf5.rpm.ReldepList(self.base)
        list2 = libdnf5.rpm.ReldepList(self.base)
        self.assertEqual(list1, list2)

        list1.add(a)
        self.assertNotEqual(list1, list2)

        list2.add(a)
        list1.add(b)
        list2.add(b)
        self.assertEqual(list1, list2)

    def test_append(self):
        a = libdnf5.rpm.Reldep(self.base, "python3-labirinto = 4.2.0")
        b = libdnf5.rpm.Reldep(self.base, "(lab-list if labirinto.txt)")
        c = libdnf5.rpm.Reldep(self.base, "(labirinto unless labirinto_c)")
        d = libdnf5.rpm.Reldep(self.base, "labirinto.txt")

        list1 = libdnf5.rpm.ReldepList(self.base)
        list1.add(a)
        list1.add(b)
        list1.add_reldep("delgado > 1.2")

        list2 = libdnf5.rpm.ReldepList(self.base)
        list2.add(c)
        list2.add(d)

        list1.append(list2)
        self.assertEqual(list1.size(), 5)
        self.assertEqual(list1.get(0), a)
        self.assertEqual(list1.get(1), b)
        self.assertEqual(list1.get(2).to_string(), "delgado > 1.2")
        self.assertEqual(list1.get(3), c)
        self.assertEqual(list1.get(4), d)

    def test_iterator(self):
        a = libdnf5.rpm.Reldep(self.base, "python3-labirinto = 4.2.0")
        b = libdnf5.rpm.Reldep(self.base, "(lab-list if labirinto.txt)")
        c = libdnf5.rpm.Reldep(self.base, "(labirinto unless labirinto_c)")
        d = libdnf5.rpm.Reldep(self.base, "labirinto.txt")
        expect = [a, b, c, d]

        list1 = libdnf5.rpm.ReldepList(self.base)
        for reldep in expect:
            list1.add(reldep)

        # check if begin() points to the first Reldep
        it1 = list1.begin()
        self.assertEqual(it1.value(), a)

        # test next()
        it1.next()
        self.assertEqual(it1.value(), b)

        # check Python style iterator
        pit = iter(list1)
        self.assertEqual(next(pit), a)
        self.assertEqual(next(pit), b)
        self.assertEqual(next(pit, None), c)
        self.assertEqual(next(pit, None), d)
        self.assertEqual(next(pit, None), None)

        # test iteration using a for loop
        result = []
        for reldep in list1:
            result.append(reldep)
        self.assertEqual(result, expect)

    # add_reldep_with_glob uses libsolvs Dataiterator which needs the actual packages
    def test_add_reldep_with_glob(self):
        list1 = libdnf5.rpm.ReldepList(self.base)
        list1.add_reldep_with_glob("pkg*")

        expected = ["pkg", "pkg.conf", "pkg.conf.d", "pkg-libs",
                    "pkg", "pkg", "pkg", "pkg", "pkg", "pkg"]
        # TODO(dmach): implement __str__()
        result = [reldep.to_string() for reldep in list1]
        self.assertEqual(expected, result)
