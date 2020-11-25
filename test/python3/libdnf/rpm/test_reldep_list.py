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


class TestReldepList(unittest.TestCase):
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
        clsSolvSack = libdnf.rpm.SolvSack
        self.sack.load_repo(repo.get(), libdnf.rpm.SolvSack.LoadRepoFlags_USE_FILELISTS |
                            libdnf.rpm.SolvSack.LoadRepoFlags_USE_OTHER |
                            libdnf.rpm.SolvSack.LoadRepoFlags_USE_PRESTO |
                            libdnf.rpm.SolvSack.LoadRepoFlags_USE_UPDATEINFO)

    def test_add(self):
        a = libdnf.rpm.Reldep(self.sack, "python3-labirinto = 4.2.0")
        b = libdnf.rpm.Reldep(self.sack, "(lab-list if labirinto.txt)")

        list1 = libdnf.rpm.ReldepList(self.sack)
        self.assertEqual(list1.size(), 0)
        list1.add(a)
        # TODO(jrohel): Shall we have add() with ReldepId as a part of public API?
        list1.add(b.get_id())
        list1.add_reldep("delgado > 1.2")
        self.assertEqual(list1.size(), 3)

    def test_get(self):
        a = libdnf.rpm.Reldep(self.sack, "python3-labirinto = 4.2.0")
        b = libdnf.rpm.Reldep(self.sack, "(lab-list if labirinto.txt)")
        c = libdnf.rpm.Reldep(self.sack, "(labirinto unless labirinto_c)")

        list1 = libdnf.rpm.ReldepList(self.sack)
        list1.add(a)
        list1.add(b)
        list1.add(c)

        self.assertEqual(list1.get(0), a)
        self.assertEqual(list1.get(1).to_string(), "(lab-list if labirinto.txt)")
        self.assertEqual(list1.get_id(2).id, c.get_id().id)

    def test_compare(self):
        a = libdnf.rpm.Reldep(self.sack, "python3-labirinto = 4.2.0")
        b = libdnf.rpm.Reldep(self.sack, "(lab-list if labirinto.txt)")

        list1 = libdnf.rpm.ReldepList(self.sack)
        list2 = libdnf.rpm.ReldepList(self.sack)
        self.assertEqual(list1, list2)

        list1.add(a)
        self.assertNotEquals(list1, list2)

        list2.add(a)
        list1.add(b)
        list2.add(b)
        self.assertEqual(list1, list2)

    def test_append(self):
        a = libdnf.rpm.Reldep(self.sack, "python3-labirinto = 4.2.0")
        b = libdnf.rpm.Reldep(self.sack, "(lab-list if labirinto.txt)")
        c = libdnf.rpm.Reldep(self.sack, "(labirinto unless labirinto_c)")
        d = libdnf.rpm.Reldep(self.sack, "labirinto.txt")

        list1 = libdnf.rpm.ReldepList(self.sack)
        list1.add(a)
        list1.add(b)
        list1.add_reldep("delgado > 1.2")

        list2 = libdnf.rpm.ReldepList(self.sack)
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
        a = libdnf.rpm.Reldep(self.sack, "python3-labirinto = 4.2.0")
        b = libdnf.rpm.Reldep(self.sack, "(lab-list if labirinto.txt)")
        c = libdnf.rpm.Reldep(self.sack, "(labirinto unless labirinto_c)")
        d = libdnf.rpm.Reldep(self.sack, "labirinto.txt")
        expect = [a, b, c, d]

        list1 = libdnf.rpm.ReldepList(self.sack)
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
        list1 = libdnf.rpm.ReldepList(self.sack)
        list1.add_reldep_with_glob("dwm*")

        expected = ["dwm-6.1-1.fc29.spec", "dwm", "dwm", "dwm(x86-64)", "dwm(x86-64)"]
        result = [reldep.to_string() for reldep in list1]
        self.assertEqual(expected, result)
