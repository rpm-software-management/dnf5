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
import tempfile
import unittest

import libdnf

import support


class TestGroup(support.LibdnfTestCase):
    def test_group(self):
        self.add_repo_repomd("repomd-comps-core")
        q_core = libdnf.comps.GroupQuery(self.base.get_comps().get_group_sack())
        core = q_core.get()
