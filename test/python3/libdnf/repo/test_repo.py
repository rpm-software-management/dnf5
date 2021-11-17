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
import tempfile
import shutil
import os

import libdnf.base
import libdnf.logger


class TestRepo(unittest.TestCase):
    def test_repo(self):
        base = libdnf.base.Base()

        # Sets path to cache directory.
        tmpdir = tempfile.mkdtemp(prefix="libdnf-python3-")
        base.get_config().cachedir().set(libdnf.conf.Option.Priority_RUNTIME, tmpdir)

        repo_sack = libdnf.repo.RepoSack(base)
        package_sack = libdnf.rpm.PackageSack(base)

        # Creates system repository and loads it
        package_sack.get_system_repo().load()

        # Creates new repositories in the repo_sack
        repo = repo_sack.new_repo("repomd-repo1")

        # Tunes repository configuration (baseurl is mandatory)
        repo_path = os.path.join(os.getcwd(), "../../../test/data/repos-repomd/repomd-repo1/")
        baseurl = "file://" + repo_path
        repo_cfg = repo.get_config()
        repo_cfg.baseurl().set(libdnf.conf.Option.Priority_RUNTIME, baseurl)

        # fetch repo metadata and load it
        repo.fetch_metadata()
        repo.load()

        # Remove the cache directory.
        shutil.rmtree(tmpdir)
