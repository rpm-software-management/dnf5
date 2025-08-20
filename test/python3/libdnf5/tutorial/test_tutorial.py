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
import pathlib
import tempfile

import base_test_case

PROJECT_BINARY_DIR = os.environ["PROJECT_BINARY_DIR"]
PROJECT_SOURCE_DIR = os.environ["PROJECT_SOURCE_DIR"]


class TestTutorial(base_test_case.BaseTestCase):
    def setUp(self):
        super().setUp()
        self.installroot = self.base.get_config().installroot
        self.cachedir = self.base.get_config().cachedir
        self.baseurl = pathlib.Path(os.path.join(
            PROJECT_BINARY_DIR, "test/data/repos-rpm/rpm-repo1/")).as_uri()

    def test_create_base(self):
        file = ""
        with open("tutorial/session/create_base.py", "r") as f:
            file += f.read()

        exec(file, {'installroot': self.installroot, 'cachedir': self.cachedir})

    def test_load_repo(self):
        file = ""
        with open("tutorial/session/create_base.py", "r") as f:
            file += f.read()

        with open("tutorial/repo/load_repo.py", "r") as f:
            file += f.read()

        exec(file, {'installroot': self.installroot,
                    'cachedir': self.cachedir, 'baseurl': self.baseurl})

    def test_load_system_repo(self):
        # TODO(nsella) This example does not 'compile' yet
        # file = ""
        # with open("tutorial/session/create_base.py", "r") as f:
        #     file += f.read()

        # with open("tutorial/repo/load_system_repos.py", "r") as f:
        #     file += f.read()

        # exec(file, { 'installroot': self.installroot, 'baseurl': self.baseurl })
        pass

    def test_query(self):
        file = ""
        with open("tutorial/session/create_base.py", "r") as f:
            file += f.read()

        with open("tutorial/repo/load_repo.py", "r") as f:
            file += f.read()

        with open("tutorial/query/query.py", "r") as f:
            file += f.read()

        exec(file, {'installroot': self.installroot,
                    'cachedir': self.cachedir, 'baseurl': self.baseurl})

    def test_transaction(self):
        file = ""
        with open("tutorial/session/create_base.py", "r") as f:
            file += f.read()

        with open("tutorial/repo/load_repo.py", "r") as f:
            file += f.read()

        with open("tutorial/transaction/transaction.py", "r") as f:
            file += f.read()

        tmp_path = os.getcwd()
        exec(file, {'installroot': self.installroot,
                    'cachedir': self.cachedir, 'baseurl': self.baseurl})
        os.chdir(tmp_path)

    def test_force_arch(self):
        file = ""
        with open("tutorial/session/force_arch.py", "r") as f:
            file += f.read()

        exec(file, {'installroot': self.installroot, 'cachedir': self.cachedir})
