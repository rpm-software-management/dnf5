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
import shutil
import tempfile
import unittest

import libdnf5.base
import libdnf5.utils


class TestBase(unittest.TestCase):
    def test_base(self):
        base = libdnf5.base.Base()
        loger = base.get_logger()
        config = base.get_config()
        repo_sack = base.get_repo_sack()
        package_sack = base.get_rpm_package_sack()

    def test_weak_ptr(self):
        # Creates a new Base object
        base = libdnf5.base.Base()

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
        # Raises an AssertionError that is not caught by the SWIG binding.
        # with self.assertRaisesRegex(libdnf5.exception.AssertionError, 'Dereferencing an invalidated WeakPtr'):
        #    vars.get_value("test_variable")
        # with self.assertRaisesRegex(libdnf5.exception.AssertionError, 'Dereferencing an invalidated WeakPtr'):
        #    vars2.get_value("test_variable")

    def test_non_existing_config_load(self):
        # Try to load configuration from non-existing path
        base = libdnf5.base.Base()
        base.get_config().config_file_path = 'this-path-does-not-exist.conf'

        # Checking the exception.
        with self.assertRaisesRegex(libdnf5.exception.MissingConfigErrorNested,
                                    'Configuration file "this-path-does-not-exist.conf" not found') as cm:
            base.load_config()
        self.assertRegex(cm.exception.format(libdnf5.exception.FormatDetailLevel_WithDomainAndName),
                         '^libdnf5::MissingConfigError: Configuration file "this-path-does-not-exist.conf" not found\n'
                         ' libdnf5::utils::fs::FileSystemError: cannot open file')

        # Checking the nested exception.
        with self.assertRaisesRegex(libdnf5.exception.FileSystemError,
                                    'cannot open file') as cm_nested:
            cm.exception.rethrow_if_nested()
        self.assertRegex(cm_nested.exception.format(libdnf5.exception.FormatDetailLevel_WithDomainAndName),
                         '^libdnf5::utils::fs::FileSystemError: cannot open file')

    def test_lock_system_repo(self):
        temp_dir = tempfile.mkdtemp(prefix="libdnf5_python3_unittest.")
        try:
            base = libdnf5.base.Base()
            config = base.get_config()
            config.installroot = os.path.join(temp_dir, "installroot")
            config.plugins = False

            self.assertIsNone(base.get_system_repo_lock())

            self.assertTrue(base.lock_system_repo(
                libdnf5.utils.LockAccess_WRITE,
                libdnf5.utils.LockBlocking_NON_BLOCKING))

            lock = base.get_system_repo_lock()
            self.assertIsNotNone(lock)
            self.assertEqual(
                libdnf5.utils.LockAccess_WRITE, lock.held_lock_access())

            base.unlock_system_repo()
            self.assertFalse(lock.holds_lock())

            self.assertTrue(base.lock_system_repo(
                libdnf5.utils.LockAccess_READ,
                libdnf5.utils.LockBlocking_BLOCKING))
            self.assertEqual(
                libdnf5.utils.LockAccess_READ,
                base.get_system_repo_lock().held_lock_access())
        finally:
            shutil.rmtree(temp_dir)
