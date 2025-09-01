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

import dbus

import support


class GoalTest(support.InstallrootCase):

    def test_do_transaction_before_resolve(self):
        with self.assertRaises(dbus.exceptions.DBusException) as cm:
            self.iface_goal.do_transaction(dbus.Dictionary({}, signature='sv'))
        self.assertEqual(str(
            cm.exception), 'org.rpm.dnf.v0.rpm.Rpm.TransactionError: Transaction has to be resolved first.')
