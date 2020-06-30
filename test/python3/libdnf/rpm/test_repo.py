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

import libdnf.base
import libdnf.logger

import os


class TestRepo(unittest.TestCase):
    def test_repo(self):
        base = libdnf.base.Base()

        # Sets path to cache directory.
        cwd = os.getcwd()
        base.get_config().cachedir().set(libdnf.conf.Option.Priority_RUNTIME, cwd)

        repo_sack = libdnf.rpm.RepoSack(base)
        solv_sack = libdnf.rpm.SolvSack(base)

        # Creates system repository and loads it into rpm::SolvSack.
        solv_sack.create_system_repo(False)

        # Creates new repositories in the repo_sack
        repo = repo_sack.new_repo("dnf-ci-fedora")

        # Tunes repositotory configuration (baseurl is mandatory)
        repo_path = os.path.join(cwd, "../../../test/libdnf/rpm/repos-data/dnf-ci-fedora/")
        baseurl = "file://" + repo_path
        repo_cfg = repo.get_config()
        repo_cfg.baseurl().set(libdnf.conf.Option.Priority_RUNTIME, baseurl)

        # Loads repository into rpm::Repo.
        repo.load()

        # Loads rpm::Repo into rpm::SolvSack
        SolvSack = libdnf.rpm.SolvSack
        solv_sack.load_repo(repo.get(), SolvSack.LoadRepoFlags_USE_PRESTO | SolvSack.LoadRepoFlags_USE_UPDATEINFO |
                            SolvSack.LoadRepoFlags_USE_OTHER)
