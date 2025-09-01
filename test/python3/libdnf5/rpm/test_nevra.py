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

import libdnf5

import base_test_case


class TestNevra(base_test_case.BaseTestCase):
    def test_nevra(self):
        nevras = libdnf5.rpm.Nevra.parse(
            "four-of-fish-8:3.6.9-11.fc100.x86_64", libdnf5.rpm.VectorNevraForm(1, libdnf5.rpm.Nevra.Form_NEVRA))
        self.assertEqual(len(nevras), 1)
        nevra = nevras.pop()
        self.assertEqual(nevra.get_name(), "four-of-fish")
        self.assertEqual(nevra.get_epoch(), "8")
        self.assertEqual(nevra.get_version(), "3.6.9")
        self.assertEqual(nevra.get_release(), "11.fc100")
        self.assertEqual(nevra.get_arch(), "x86_64")

        # test that to_nevra_string() and to_full_nevra_string() template functions work
        self.assertEqual(
            libdnf5.rpm.to_nevra_string(nevra), "four-of-fish-8:3.6.9-11.fc100.x86_64")
        self.assertEqual(
            libdnf5.rpm.to_full_nevra_string(nevra), "four-of-fish-8:3.6.9-11.fc100.x86_64")

    def test_nevra_without_epoch(self):
        nevras = libdnf5.rpm.Nevra.parse(
            "four-of-fish-3.6.9-11.fc100.x86_64", libdnf5.rpm.VectorNevraForm(1, libdnf5.rpm.Nevra.Form_NEVRA))
        self.assertEqual(len(nevras), 1)
        nevra = nevras.pop()
        self.assertEqual(nevra.get_name(), "four-of-fish")
        self.assertEqual(nevra.get_epoch(), "")
        self.assertEqual(nevra.get_version(), "3.6.9")
        self.assertEqual(nevra.get_release(), "11.fc100")
        self.assertEqual(nevra.get_arch(), "x86_64")

        # test that to_nevra_string() and to_full_nevra_string() template functions work without epoch
        self.assertEqual(
            libdnf5.rpm.to_nevra_string(nevra), "four-of-fish-3.6.9-11.fc100.x86_64")
        self.assertEqual(
            libdnf5.rpm.to_full_nevra_string(nevra), "four-of-fish-0:3.6.9-11.fc100.x86_64")

    def test_nevra_with_colon_in_name(self):
        nevras = libdnf5.rpm.Nevra.parse(
            "four-of-f:ish-3.6.9-11.fc100.x86_64", libdnf5.rpm.VectorNevraForm(1, libdnf5.rpm.Nevra.Form_NEVRA))
        self.assertEqual(len(nevras), 0)

    def test_nevra_with_two_colons(self):
        with self.assertRaises(libdnf5.exception.RpmNevraIncorrectInputError):
            libdnf5.rpm.Nevra.parse(
                "four-of-fish-8:9:3.6.9-11.fc100.x86_64", libdnf5.rpm.VectorNevraForm(1, libdnf5.rpm.Nevra.Form_NEVRA))

    def test_nevra_form_NEV(self):
        nevras = libdnf5.rpm.Nevra.parse(
            "four-of-fish-8:3.6.9", libdnf5.rpm.VectorNevraForm(1, libdnf5.rpm.Nevra.Form_NEV))
        self.assertEqual(len(nevras), 1)
        nevra = nevras.pop()
        self.assertEqual(nevra.get_name(), "four-of-fish")
        self.assertEqual(nevra.get_epoch(), "8")
        self.assertEqual(nevra.get_version(), "3.6.9")
        self.assertEqual(nevra.get_release(), "")
        self.assertEqual(nevra.get_arch(), "")

        nevras = libdnf5.rpm.Nevra.parse(
            "fish-8:3.6.9", libdnf5.rpm.VectorNevraForm(1, libdnf5.rpm.Nevra.Form_NEV))
        self.assertEqual(len(nevras), 1)
        nevra = nevras.pop()
        self.assertEqual(nevra.get_name(), "fish")
        self.assertEqual(nevra.get_epoch(), "8")
        self.assertEqual(nevra.get_version(), "3.6.9")
        self.assertEqual(nevra.get_release(), "")
        self.assertEqual(nevra.get_arch(), "")

        nevras = libdnf5.rpm.Nevra.parse(
            "fish-3.6.9", libdnf5.rpm.VectorNevraForm(1, libdnf5.rpm.Nevra.Form_NEV))
        self.assertEqual(len(nevras), 1)
        nevra = nevras.pop()
        self.assertEqual(nevra.get_name(), "fish")
        self.assertEqual(nevra.get_epoch(), "")
        self.assertEqual(nevra.get_version(), "3.6.9")
        self.assertEqual(nevra.get_release(), "")
        self.assertEqual(nevra.get_arch(), "")

        nevras = libdnf5.rpm.Nevra.parse(
            "four-of-fish-3.6.9.i686", libdnf5.rpm.VectorNevraForm(1, libdnf5.rpm.Nevra.Form_NEV))
        self.assertEqual(len(nevras), 1)
        nevra = nevras.pop()
        self.assertEqual(nevra.get_name(), "four-of-fish")
        self.assertEqual(nevra.get_epoch(), "")
        self.assertEqual(nevra.get_version(), "3.6.9.i686")
        self.assertEqual(nevra.get_release(), "")
        self.assertEqual(nevra.get_arch(), "")

    def test_nevra_form_NA(self):
        nevras = libdnf5.rpm.Nevra.parse(
            "four-of-fish-3.6.9.i686", libdnf5.rpm.VectorNevraForm(1, libdnf5.rpm.Nevra.Form_NA))
        self.assertEqual(len(nevras), 1)
        nevra = nevras.pop()
        self.assertEqual(nevra.get_name(), "four-of-fish-3.6.9")
        self.assertEqual(nevra.get_epoch(), "")
        self.assertEqual(nevra.get_version(), "")
        self.assertEqual(nevra.get_release(), "")
        self.assertEqual(nevra.get_arch(), "i686")

    def test_nevra_incorrect_form_NA(self):
        nevras = libdnf5.rpm.Nevra.parse(
            "name.ar-ch", libdnf5.rpm.VectorNevraForm(1, libdnf5.rpm.Nevra.Form_NA))
        self.assertEqual(len(nevras), 0)

        nevras = libdnf5.rpm.Nevra.parse(
            "name.-arch", libdnf5.rpm.VectorNevraForm(1, libdnf5.rpm.Nevra.Form_NA))
        self.assertEqual(len(nevras), 0)

        nevras = libdnf5.rpm.Nevra.parse(
            "name.", libdnf5.rpm.VectorNevraForm(1, libdnf5.rpm.Nevra.Form_NA))
        self.assertEqual(len(nevras), 0)

        nevras = libdnf5.rpm.Nevra.parse(
            "name", libdnf5.rpm.VectorNevraForm(1, libdnf5.rpm.Nevra.Form_NA))
        self.assertEqual(len(nevras), 0)

    def test_nevra_invalid_characters(self):
        with self.assertRaises(libdnf5.exception.RpmNevraIncorrectInputError):
            nevras = libdnf5.rpm.Nevra.parse(
                "four-of(fish.i686)", libdnf5.rpm.VectorNevraForm(1, libdnf5.rpm.Nevra.Form_NA))

    def test_nevra_with_glob_in_epoch(self):
        nevras = libdnf5.rpm.Nevra.parse(
            "four-of-fish-[01]:3.6.9-11.fc100.x86_64", libdnf5.rpm.VectorNevraForm(1, libdnf5.rpm.Nevra.Form_NEVRA))
        self.assertEqual(len(nevras), 1)
        nevra = nevras.pop()
        self.assertEqual(nevra.get_name(), "four-of-fish")
        self.assertEqual(nevra.get_epoch(), "[01]")
        self.assertEqual(nevra.get_version(), "3.6.9")
        self.assertEqual(nevra.get_release(), "11.fc100")
        self.assertEqual(nevra.get_arch(), "x86_64")

        nevras = libdnf5.rpm.Nevra.parse(
            "four-of-fish-?:3.6.9-11.fc100.x86_64", libdnf5.rpm.VectorNevraForm(1, libdnf5.rpm.Nevra.Form_NEVRA))
        self.assertEqual(len(nevras), 1)
        nevra = nevras.pop()
        self.assertEqual(nevra.get_name(), "four-of-fish")
        self.assertEqual(nevra.get_epoch(), "?")
        self.assertEqual(nevra.get_version(), "3.6.9")
        self.assertEqual(nevra.get_release(), "11.fc100")
        self.assertEqual(nevra.get_arch(), "x86_64")
