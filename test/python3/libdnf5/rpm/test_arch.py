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
import unittest

import libdnf5

import base_test_case


class TestRpmArch(unittest.TestCase):
    def test_get_supported_arches(self):
        arches = libdnf5.rpm.get_supported_arches()
        # at least one architecture was found
        self.assertTrue(len(arches) > 0)
        # the arches list is sorted and contains unique elements
        for i in range(len(arches) - 1):
            self.assertTrue(arches[i] < arches[i+1])

    def test_get_base_arch(self):
        self.assertEqual(libdnf5.rpm.get_base_arch("x86_64"), "x86_64")
        self.assertEqual(libdnf5.rpm.get_base_arch("amd64"), "x86_64")
        self.assertEqual(libdnf5.rpm.get_base_arch("UNKNOWN_ARCH"), "")
