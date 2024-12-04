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

import libdnf5

import base_test_case


class TestVars(base_test_case.BaseTestCase):
    def test_getting_undefined_variable(self):
        vars = self.base.get_vars()
        self.assertRaises(IndexError, vars.get_value, "undefined")

    def test_detect_release(self):
        installroot = self.base.get_config().installroot
        # Cannot detect release in nonexistent directory, return None
        release = libdnf5.conf.Vars.detect_release(
            self.base.get_weak_ptr(), os.path.join(installroot, "nonexist"))
        self.assertEqual(release, None)
