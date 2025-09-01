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


import os
import shutil
import tempfile
import unittest

import libdnf5


PROJECT_BINARY_DIR = os.environ["PROJECT_BINARY_DIR"]
PROJECT_SOURCE_DIR = os.environ["PROJECT_SOURCE_DIR"]


class BaseTestCase(unittest.TestCase):
    def setUp(self):
        self.base = libdnf5.base.Base()

        self.temp_dir = tempfile.mkdtemp(prefix="libdnf5_python3_unittest.")

        config = self.base.get_config()
        config.installroot = os.path.join(self.temp_dir, "installroot")
        config.cachedir = os.path.join(self.temp_dir, "cache")
        config.optional_metadata_types = libdnf5.conf.OPTIONAL_METADATA_TYPES

        # Prevent loading plugins from the host
        config.plugins = False

        vars = self.base.get_vars().get()
        vars.set("arch", "x86_64")

        self.base.setup()

        self.repo_sack = self.base.get_repo_sack()
        self.package_sack = self.base.get_rpm_package_sack()

    def tearDown(self):
        shutil.rmtree(self.temp_dir)

    def _add_repo(self, repoid, repo_path, load=True):
        """
        Add a repo from `repo_path`.
        """
        repo = self.repo_sack.create_repo(repoid)
        repo.get_config().baseurl = "file://" + repo_path

        if load:
            self.repo_sack.load_repos(libdnf5.repo.Repo.Type_AVAILABLE)

        return repo

    def add_repo_repomd(self, repoid, load=True):
        """
        Add a repo from PROJECT_SOURCE_DIR/test/data/repos-repomd/<repoid>/repodata
        """
        repo_path = os.path.join(
            PROJECT_SOURCE_DIR, "test/data/repos-repomd", repoid)
        return self._add_repo(repoid, repo_path, load)

    def add_repo_rpm(self, repoid, load=True):
        """
        Add a repo from PROJECT_BINARY_DIR/test/data/repos-rpm/<repoid>/repodata
        """
        repo_path = os.path.join(
            PROJECT_BINARY_DIR, "test/data/repos-rpm", repoid)
        return self._add_repo(repoid, repo_path, load)

    def add_repo_solv(self, repoid):
        """
        Add a repo from PROJECT_SOURCE_DIR/test/data/repos-solv/<repoid>.repo
        """
        repo_path = os.path.join(
            PROJECT_SOURCE_DIR, "test/data/repos-solv", repoid + ".repo")
        return self.repo_sack.create_repo_from_libsolv_testcase(repoid, repo_path)
