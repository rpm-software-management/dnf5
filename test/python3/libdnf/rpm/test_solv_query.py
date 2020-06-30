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
        self.assertEqual(query.size(), 289)

    def test_ifilter_name(self):

        nevras = {"CQRlib-1.1.1-4.fc29.src", "CQRlib-1.1.1-4.fc29.x86_64"}
        nevras_contains = {"CQRlib-1.1.1-4.fc29.src", "CQRlib-1.1.1-4.fc29.x86_64",
                           "CQRlib-devel-1.1.2-16.fc29.src", "CQRlib-devel-1.1.2-16.fc29.x86_64"}
        full_nevras = {"CQRlib-0:1.1.1-4.fc29.src", "CQRlib-0:1.1.1-4.fc29.x86_64",
                       "nodejs-1:5.12.1-1.fc29.src", "nodejs-1:5.12.1-1.fc29.x86_64"}

        # Test QueryCmp::EQ
        query = libdnf.rpm.SolvQuery(self.sack)
        names = ["CQRlib"]
        query.ifilter_name(libdnf.common.QueryCmp_EQ, libdnf.common.VectorString(names))
        self.assertEqual(query.size(), 2)
        pset = query.get_package_set()
        self.assertEqual(pset.size(), 2)
        for pkg in pset:
            self.assertTrue(pkg.get_nevra() in nevras)

        # Test QueryCmp::GLOB
        query2 = libdnf.rpm.SolvQuery(self.sack)
        names2 = ["CQ?lib"]
        query2.ifilter_name(libdnf.common.QueryCmp_GLOB, libdnf.common.VectorString(names2))
        self.assertEqual(query2.size(), 2)
        pset2 = query2.get_package_set()
        for pkg in pset2:
            self.assertTrue(pkg.get_nevra() in nevras)
