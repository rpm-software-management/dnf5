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

import gc
import unittest

import libdnf5


class TestLifetime(unittest.TestCase):
    def test_config_keeps_base_alive(self):
        base = libdnf5.base.Base()
        config = base.get_config()
        expected = config.cachedir

        del base
        gc.collect()

        self.assertEqual(config.cachedir, expected)

    def test_config_from_temporary_base(self):
        expected = libdnf5.base.Base().get_config().cachedir
        self.assertEqual(
            libdnf5.base.Base().get_config().cachedir, expected)

    def test_config_stored_from_temporary_base(self):
        config = libdnf5.base.Base().get_config()
        gc.collect()
        self.assertTrue(config.cachedir)

    def test_option_object_keeps_owners_alive(self):
        option = libdnf5.base.Base().get_config().get_cachedir_option()
        gc.collect()
        self.assertTrue(option.get_value())

    def test_vars_weak_ptr_keeps_base_alive(self):
        vars = libdnf5.base.Base().get_vars()
        gc.collect()
        self.assertIsNotNone(vars.get())


if __name__ == "__main__":
    unittest.main()
