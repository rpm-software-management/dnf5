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


class TestGroup(unittest.TestCase):
    def test_group(self):
        base = libdnf.base.Base()

        # Sets path to cache directory.
        tmpdir = tempfile.mkdtemp(prefix="libdnf-python3-test-comps-")
        base.get_config().cachedir().set(libdnf.conf.Option.Priority_RUNTIME, tmpdir)

        repo = base.get_repo_sack().new_repo("repo")
        comps = libdnf.comps.Comps(base)
        data_path = os.path.join(os.getcwd(), "../../../test/libdnf/comps/data/core.xml")
        comps.load_from_file(repo, data_path)
        q_core = libdnf.comps.GroupQuery(comps.get_group_sack())
        core = q_core.get()
