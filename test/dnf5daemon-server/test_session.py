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

import dbus
import unittest

import support


class SessionTest(unittest.TestCase):

    def setUp(self):
        self.bus = dbus.SystemBus()
        self.proxy = self.bus.get_object(support.DNFDAEMON_BUS_NAME,
                                         support.DNFDAEMON_OBJECT_PATH)
        self.iface = dbus.Interface(self.proxy,
                                    dbus_interface=support.IFACE_SESSION_MANAGER)

    def tearDown(self):
        pass

    def test_session(self):
        session = self.iface.open_session({})
        # session address has expected format
        self.assertRegex(
            session, r'^%s/[0-9a-f]{32}$' % support.DNFDAEMON_OBJECT_PATH)
        # close the session
        self.assertEqual(dbus.Boolean(True), self.iface.close_session(session))
        # closing non-existent session returns False
        self.assertEqual(dbus.Boolean(False),
                         self.iface.close_session(session))
