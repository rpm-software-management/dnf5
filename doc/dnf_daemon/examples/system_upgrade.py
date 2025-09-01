# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later
#!/usr/bin/python3

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

"""
This is an example how to perform a system-upgrade with dnf5daemon-server.
"""

import dbus

DNFDAEMON_BUS_NAME = 'org.rpm.dnf.v0'
DNFDAEMON_OBJECT_PATH = '/' + DNFDAEMON_BUS_NAME.replace('.', '/')

IFACE_SESSION_MANAGER = '{}.SessionManager'.format(DNFDAEMON_BUS_NAME)
IFACE_RPM = '{}.rpm.Rpm'.format(DNFDAEMON_BUS_NAME)
IFACE_GOAL = '{}.Goal'.format(DNFDAEMON_BUS_NAME)


bus = dbus.SystemBus()
iface_session = dbus.Interface(
    bus.get_object(DNFDAEMON_BUS_NAME, DNFDAEMON_OBJECT_PATH),
    dbus_interface=IFACE_SESSION_MANAGER)

# set the releasever to the new distribution release
session = iface_session.open_session(
    dbus.Dictionary({"releasever": "40"}, signature=dbus.Signature('sv')))

iface_rpm = dbus.Interface(
    bus.get_object(DNFDAEMON_BUS_NAME, session),
    dbus_interface=IFACE_RPM)
iface_goal = dbus.Interface(
    bus.get_object(DNFDAEMON_BUS_NAME, session),
    dbus_interface=IFACE_GOAL)

# Add system upgrade to the transaction
options = {
    "mode": "distrosync",
}
iface_rpm.system_upgrade(options)

# resolve the transaction
resolved, result = iface_goal.resolve({})

# now you can print the transaction table and ask the user for confirmation
print("Resolved.")

if result == 0:
    # execute the transaction offline (durint the next reboot)
    iface_goal.do_transaction({"offline": True}, timeout=2000)
    print("Reboot to continue with system upgrade...")
else:
    errors = iface_goal.get_transaction_problems_string()
    print("Errors while resolving the transaction:")
    for error in errors:
        print(error)
