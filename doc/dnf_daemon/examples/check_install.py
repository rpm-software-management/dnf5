#!/usr/bin/python3
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

import dbus


def human_readable_size(size):
    for unit in ['B', 'KiB', 'MiB', 'GiB']:
        if size < 1024.0 or unit == 'GiB':
            break
        size /= 1024.0
    return f"{size:.2f} {unit}"


DNFDAEMON_BUS_NAME = 'org.rpm.dnf.v0'
DNFDAEMON_OBJECT_PATH = '/' + DNFDAEMON_BUS_NAME.replace('.', '/')

IFACE_SESSION_MANAGER = '{}.SessionManager'.format(DNFDAEMON_BUS_NAME)
IFACE_RPM = '{}.rpm.Rpm'.format(DNFDAEMON_BUS_NAME)


bus = dbus.SystemBus()
iface_session = dbus.Interface(
    bus.get_object(DNFDAEMON_BUS_NAME, DNFDAEMON_OBJECT_PATH),
    dbus_interface=IFACE_SESSION_MANAGER)

# set the releasever to the new distribution release
session = iface_session.open_session(
    dbus.Dictionary({}, signature=dbus.Signature('sv')))

iface_rpm = dbus.Interface(
    bus.get_object(DNFDAEMON_BUS_NAME, session),
    dbus_interface=IFACE_RPM)

# Add system upgrade to the transaction
options = {
}
packages = iface_rpm.check_install("0ad", options)

print("Following packages will be downloaded and installed:")
total_download_size = 0
total_install_size = 0
for pkg in packages:
    print(pkg["install_reason"], pkg["name"], pkg["evr"])
    print("  download size: {}, install size: {}".format(human_readable_size(
        pkg["download_size"]), human_readable_size(pkg["install_size"])))
    total_download_size += pkg["download_size"]
    total_install_size += pkg["install_size"]

print("Need to download {}.".format(human_readable_size(total_download_size)))
print("Instalation will require additional {}.".format(
    human_readable_size(total_install_size)))
