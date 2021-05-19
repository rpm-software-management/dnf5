# Copyright (C) 2020-2021 Red Hat, Inc.
#
# This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
#
# Libdnf is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Libdnf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

import unittest

import libdnf.base


class TestBase(unittest.TestCase):
    def test_base(self):
        base = libdnf.base.Base()
        loger = base.get_logger()
        config = base.get_config()
        repo_sack = base.get_rpm_repo_sack()
        solv_sack = base.get_rpm_solv_sack()

    def test_weak_ptr(self):
        # Creates a new Base object
        base = libdnf.base.Base()

        # Gets a WeakPtr pointing to Vars in the Base object
        vars = base.get_vars()

        # Creates a copy of WeakPtr
        vars2 = vars

        # Base is valid -> WeakPtr is valid. Sets "test_variable" using WeakPtr vars.
        vars.set("test_variable", "value1")

        # Base is valid -> WeakPtr is valid. Gets value of "test_variable" using copy of WeakPtr vars2.
        self.assertEqual(vars2.get_value("test_variable"), "value1")

        # Invalidates Base object
        base = None

        # Base object is invalid. -> Both WeakPtr are invalid. The code must throw an exception.
        with self.assertRaisesRegex(RuntimeError, '.*InvalidPtr.*'):
            vars.get_value("test_variable")
        with self.assertRaisesRegex(RuntimeError, '.*InvalidPtr.*'):
            vars2.get_value("test_variable")
