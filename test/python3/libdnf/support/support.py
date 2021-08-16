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


import os
import shutil
import tempfile
import unittest

import libdnf


PROJECT_BINARY_DIR = os.environ["PROJECT_BINARY_DIR"]
PROJECT_SOURCE_DIR = os.environ["PROJECT_SOURCE_DIR"]


class LibdnfTestCase(unittest.TestCase):
    def setUp(self):
        self.base = libdnf.base.Base()

        self.cachedir = tempfile.mkdtemp(prefix="libdnf-test-python3-")
        self.base.get_config().cachedir().set(libdnf.conf.Option.Priority_RUNTIME, self.cachedir)

        self.repo_sack = self.base.get_repo_sack()
        self.sack = self.base.get_rpm_package_sack()

    def tearDown(self):
        shutil.rmtree(self.cachedir)

    """
    Add (load) a repo from `repo_path`.
    It's also a shared code for add_repo_repomd() and add_repo_rpm().
    """
    def _add_repo(self, repoid, repo_path):
        repo = self.repo_sack.new_repo(repoid)

        # set the repo baseurl
        repo.get_config().baseurl().set(libdnf.conf.Option.Priority_RUNTIME, "file://" + repo_path)

        # load repository into rpm.Repo
        repo.load()

        # load repo content into rpm.PackageSack
        self.sack.load_repo(repo.get())

    """
    Add (load) a repo from PROJECT_SOURCE_DIR/test/data/repos-repomd/<repoid>/repodata
    """
    def add_repo_repomd(self, repoid):
        repo_path = os.path.join(PROJECT_SOURCE_DIR, "test/data/repos-repomd", repoid)
        self._add_repo(repoid, repo_path)

    """
    Add (load) a repo from PROJECT_BINARY_DIR/test/data/repos-rpm/<repoid>/repodata
    """
    def add_repo_rpm(self, repoid):
        repo_path = os.path.join(PROJECT_BINARY_DIR, "test/data/repos-rpm", repoid)
        self._add_repo(repoid, repo_path)


    """
    Add (load) a repo from PROJECT_SOURCE_DIR/test/data/repos-solv/<repoid>.repo
    """
    def add_repo_solv(self, repoid):
        repo_path = os.path.join(PROJECT_SOURCE_DIR, "test/data/repos-solv", repoid + ".repo")
        self.repo_sack.new_repo_from_libsolv_testcase(repoid, repo_path)
