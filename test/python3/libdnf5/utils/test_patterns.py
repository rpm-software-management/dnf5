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

import unittest

import libdnf5.utils


class TestIsXPattern(unittest.TestCase):
    def test_is_file_pattern(self):
        self.assertFalse(libdnf5.utils.is_file_pattern(''))
        self.assertFalse(libdnf5.utils.is_file_pattern('no_file_pattern'))
        self.assertFalse(libdnf5.utils.is_file_pattern('no_file/pattern'))
        self.assertTrue(libdnf5.utils.is_file_pattern('/pattern'))
        self.assertTrue(libdnf5.utils.is_file_pattern('*/pattern'))

    def test_is_glob_pattern(self):
        self.assertFalse(libdnf5.utils.is_glob_pattern(''))
        self.assertFalse(libdnf5.utils.is_glob_pattern('no_glob_pattern'))
        self.assertTrue(libdnf5.utils.is_glob_pattern('glob*_pattern'))
        self.assertTrue(libdnf5.utils.is_glob_pattern('glob[sdf]_pattern'))
        self.assertTrue(libdnf5.utils.is_glob_pattern('glob?_pattern'))
