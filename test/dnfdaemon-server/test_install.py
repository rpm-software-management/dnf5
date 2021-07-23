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

import support

class InstallTest(support.InstallrootCase):

    def test_install_package(self):
        # install a package
        self.iface_rpm.install(['one'], dbus.Dictionary({}, signature='sv'))

        resolved = self.iface_goal.resolve(dbus.Dictionary({}, signature='sv'))

        # id of package depends on order of the repos in the sack which varies
        # between runs so we can't rely on the value
        for action, pkg in resolved:
            pkg.pop('id')
            pkg.pop('package_size')

        self.assertCountEqual(
            resolved,
            dbus.Array([
                dbus.Struct((
                    dbus.UInt32(1),   # action
                    dbus.Dictionary({ # package
                        dbus.String('arch'): dbus.String('noarch', variant_level=1),
                        dbus.String('epoch'): dbus.String('0', variant_level=1),
                        dbus.String('evr'): dbus.String('2-1', variant_level=1),
                        dbus.String('name'): dbus.String('one', variant_level=1),
                        dbus.String('install_size'): dbus.UInt64(0, variant_level=1),
                        dbus.String('release'): dbus.String('1', variant_level=1),
                        dbus.String('repo'): dbus.String('rpm-repo1', variant_level=1),
                        dbus.String('version'): dbus.String('2', variant_level=1)
                        }, signature=dbus.Signature('sv'))),
                    signature=None)
                ], signature=dbus.Signature('(ua{sv})'))
            )

        self.iface_goal.do_transaction(dbus.Dictionary({}, signature='sv'))

    def test_install_strict(self):
        '''with strict=True attempt to install nonexistent package returns error'''
        self.iface_rpm.install(['no_one'], dbus.Dictionary({'strict': True}, signature='sv'))
        with self.assertRaises(dbus.exceptions.DBusException) as exc:
            resolved = self.iface_goal.resolve(dbus.Dictionary({}, signature='sv'))

    def test_install_nostrict(self):
        '''
        with strict=True attempt to install nonexistent package returns empty
        transaction
        '''
        self.iface_rpm.install(['no_one'], dbus.Dictionary({'strict': False}, signature='sv'))
        self.assertCountEqual(
            self.iface_goal.resolve(dbus.Dictionary({}, signature='sv')),
            dbus.Array([
                ], signature=dbus.Signature('(ua{sv})'))
            )

    def test_install_fromrepo(self):
        '''
        attempt to install package from repo that does not contain it return error
        '''
        self.iface_rpm.install(['one'], dbus.Dictionary({'repo_ids': ['rpm-repo2']}, signature='sv'))
        with self.assertRaises(dbus.exceptions.DBusException) as exc:
            self.iface_goal.resolve(dbus.Dictionary({}, signature='sv')),
