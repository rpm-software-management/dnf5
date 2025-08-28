# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later

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
