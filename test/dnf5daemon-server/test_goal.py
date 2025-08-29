# Copyright Contributors to the DNF5 project.
# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later

import dbus

import support


class GoalTest(support.InstallrootCase):

    def test_do_transaction_before_resolve(self):
        with self.assertRaises(dbus.exceptions.DBusException) as cm:
            self.iface_goal.do_transaction(dbus.Dictionary({}, signature='sv'))
        self.assertEqual(str(
            cm.exception), 'org.rpm.dnf.v0.rpm.Rpm.TransactionError: Transaction has to be resolved first.')
