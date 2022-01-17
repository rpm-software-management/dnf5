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

import libdnf.base

import base_test_case


class TestRepo(base_test_case.BaseTestCase):
    def test_load_system_repo(self):
        # TODO(lukash) there's no rpmdb in the installroot, create data for the test
        self.repo_sack.get_system_repo().load()


    def test_repo(self):
        repo = self.add_repo_repomd("repomd-repo1", load=False)

        repo.fetch_metadata()
        repo.load()
