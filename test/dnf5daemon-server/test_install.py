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

        resolved, result = self.iface_goal.resolve(
            dbus.Dictionary({}, signature='sv'))
        self.sanitize_transaction(resolved)

        self.assertEqual(result, 0)
        self.assertCountEqual(
            resolved,
            dbus.Array([
                dbus.Struct((
                    dbus.String('Package'),     # object type
                    dbus.String('Install'),     # action
                    dbus.String('User'),        # reason
                    dbus.Dictionary({           # transaction item attrs
                    }, signature=dbus.Signature('sv')),
                    dbus.Dictionary({           # package
                        dbus.String('full_nevra'): dbus.String('one-0:2-1.noarch', variant_level=1),
                        dbus.String('arch'): dbus.String('noarch', variant_level=1),
                        dbus.String('epoch'): dbus.String('0', variant_level=1),
                        dbus.String('evr'): dbus.String('2-1', variant_level=1),
                        dbus.String('name'): dbus.String('one', variant_level=1),
                        dbus.String('install_size'): dbus.UInt64(0, variant_level=1),
                        dbus.String('release'): dbus.String('1', variant_level=1),
                        dbus.String('repo_id'): dbus.String('rpm-repo1', variant_level=1),
                        dbus.String('version'): dbus.String('2', variant_level=1),
                        dbus.String('from_repo_id'): dbus.String('', variant_level=1),
                        dbus.String('reason'): dbus.String('None', variant_level=1),
                    }, signature=dbus.Signature('sv'))),
                    signature=None)
            ], signature=dbus.Signature('(ua{sv})'))
        )

        self.iface_goal.do_transaction(dbus.Dictionary({}, signature='sv'))

    def test_install_no_skip_unavailable(self):
        '''with skip_unavailable=False attempt to install nonexistent package returns error'''
        self.iface_rpm.install(['no_one'], dbus.Dictionary(
            {'skip_unavailable': False}, signature='sv'))
        resolved, result = self.iface_goal.resolve(
            dbus.Dictionary({}, signature='sv'))

        self.assertEqual(result, 2)
        self.assertCountEqual(
            resolved,
            dbus.Array([
            ], signature=dbus.Signature('(ua{sv})'))
        )

        errors = self.iface_goal.get_transaction_problems_string()
        self.assertEqual(errors, dbus.Array([
            dbus.String('No match for argument: no_one')], signature=dbus.Signature('s')))

    def test_install_skip_unavailable(self):
        '''
        with skip_unavailable=True attempt to install nonexistent package returns empty
        transaction
        '''
        self.iface_rpm.install(['no_one'], dbus.Dictionary(
            {'skip_unavailable': True}, signature='sv'))

        resolved, result = self.iface_goal.resolve(
            dbus.Dictionary({}, signature='sv'))

        self.assertEqual(result, 1)
        self.assertCountEqual(
            resolved,
            dbus.Array([
            ], signature=dbus.Signature('(ua{sv})'))
        )

        errors = self.iface_goal.get_transaction_problems_string()
        self.assertEqual(errors, dbus.Array([
            dbus.String('No match for argument: no_one')], signature=dbus.Signature('s')))

    def test_install_fromrepo(self):
        '''
        attempt to install package from repo that does not contain it return error
        '''
        self.iface_rpm.install(['one'], dbus.Dictionary(
            {'repo_ids': ['rpm-repo2']}, signature='sv'))
        resolved, result = self.iface_goal.resolve(
            dbus.Dictionary({}, signature='sv'))

        self.assertEqual(result, 2)
        self.assertCountEqual(
            resolved,
            dbus.Array([
            ], signature=dbus.Signature("(ua{sv})")),
        )

        errors = self.iface_goal.get_transaction_problems_string()
        self.assertEqual(errors, dbus.Array([
            dbus.String("No match for argument 'one' in repositories 'rpm-repo2'")],
            signature=dbus.Signature('s')))

    def test_install_file(self):
        '''
        Test installation using rpm file on the disk.
        '''
        self.iface_rpm.install([self.path_to_repo_rpm(
            "rpm-repo1", "one-1-1.noarch.rpm")], dbus.Dictionary({}, signature='sv'))

        resolved, result = self.iface_goal.resolve(
            dbus.Dictionary({}, signature='sv'))
        self.sanitize_transaction(resolved)

        self.assertEqual(result, 0)
        self.assertCountEqual(
            resolved,
            dbus.Array([
                dbus.Struct((
                    dbus.String('Package'),     # object type
                    dbus.String('Install'),     # action
                    dbus.String('User'),        # reason
                    dbus.Dictionary({           # transaction item attrs
                    }, signature=dbus.Signature('sv')),
                    dbus.Dictionary({           # package
                        dbus.String('full_nevra'): dbus.String('one-0:1-1.noarch', variant_level=1),
                        dbus.String('arch'): dbus.String('noarch', variant_level=1),
                        dbus.String('epoch'): dbus.String('0', variant_level=1),
                        dbus.String('evr'): dbus.String('1-1', variant_level=1),
                        dbus.String('name'): dbus.String('one', variant_level=1),
                        dbus.String('install_size'): dbus.UInt64(0, variant_level=1),
                        dbus.String('release'): dbus.String('1', variant_level=1),
                        dbus.String('repo_id'): dbus.String('@commandline', variant_level=1),
                        dbus.String('version'): dbus.String('1', variant_level=1),
                        dbus.String('from_repo_id'): dbus.String('', variant_level=1),
                        dbus.String('reason'): dbus.String('None', variant_level=1),
                    }, signature=dbus.Signature('sv'))),
                    signature=None)
            ], signature=dbus.Signature('(ua{sv})'))
        )

        self.iface_goal.do_transaction(dbus.Dictionary({}, signature='sv'))
