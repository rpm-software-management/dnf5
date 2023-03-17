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

import libdnf5.base


class TestGoal(unittest.TestCase):
    def test_missing_setup_goal_resolve(self):
        # Create a new Base object
        base = libdnf5.base.Base()

        # Create a new empty Goal
        goal = libdnf5.base.Goal(base)

        # Try to resolve the goal without running base.setup()
        self.assertRaises(RuntimeError, goal.resolve)

    def test_missing_rpm_goal_resolve(self):
        # Try to resolve the goal using unknown RPM package
        base = libdnf5.base.Base()
        base.setup()

        goal = libdnf5.base.Goal(base)
        goal.add_install('unknown_pkg.rpm')

        self.assertRaises(RuntimeError, goal.resolve)

    def test_invalid_url_goal_resolve(self):
        # Try to resolve the goal using invalid URL
        base = libdnf5.base.Base()
        base.setup()

        goal = libdnf5.base.Goal(base)
        goal.add_install('https://i-dont-exist.com')

        self.assertRaises(RuntimeError, goal.resolve)

    def test_unsupported_argument_add_remove(self):
        # Try passing unsupported argument to add_remove() method
        base = libdnf5.base.Base()
        base.setup()

        goal = libdnf5.base.Goal(base)

        self.assertRaises(RuntimeError, goal.add_remove, 'pkg.rpm')

    def test_group_rpm_reason_change_without_id(self):
        # Try changing group reason without group_id
        base = libdnf5.base.Base()
        base.setup()

        goal = libdnf5.base.Goal(base)

        self.assertRaises(RuntimeError, goal.add_rpm_reason_change, '@fake-group-spec', libdnf5.base.transaction.TransactionItemReason_GROUP, '')
