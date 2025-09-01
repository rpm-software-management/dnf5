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
import os
import subprocess

import support


class DistroSyncTest(support.InstallrootCase):

    def setUp(self):
        super(DistroSyncTest, self).setUp()
        # install a package inside the installroot
        pkg_file = os.path.join(
            support.PROJECT_BINARY_DIR, "test/data/repos-rpm/rpm-repo1/one-1-1.noarch.rpm")
        res = subprocess.run(
            ["rpm", "--root", self.installroot, "-U", pkg_file])
        self.assertEqual(
            res.returncode, 0, "Installation of test package '{}' failed.".format(pkg_file))

    def test_distro_sync_package(self):
        # remove an installed package
        self.iface_rpm.distro_sync(
            ['one'], dbus.Dictionary({}, signature='sv'))

        resolved, errors = self.iface_goal.resolve(
            dbus.Dictionary({}, signature='sv'))
        self.sanitize_transaction(resolved)

        self.assertCountEqual(
            resolved,
            dbus.Array([
                dbus.Struct((
                    dbus.String('Package'),         # object type
                    dbus.String('Upgrade'),         # action
                    dbus.String('External User'),   # reason
                    dbus.Dictionary({               # transaction item attrs
                    }, signature=dbus.Signature('sv')),
                    dbus.Dictionary({               # package
                        dbus.String('full_nevra'): dbus.String('one-0:2-1.noarch', variant_level=1),
                        dbus.String('evr'): dbus.String('2-1', variant_level=1),
                        dbus.String('name'): dbus.String('one', variant_level=1),
                        dbus.String('epoch'): dbus.String('0', variant_level=1),
                        dbus.String('version'): dbus.String('2', variant_level=1),
                        dbus.String('release'): dbus.String('1', variant_level=1),
                        dbus.String('arch'): dbus.String('noarch', variant_level=1),
                        dbus.String('install_size'): dbus.UInt64(0, variant_level=1),
                        dbus.String('repo_id'): dbus.String('rpm-repo1', variant_level=1),
                        dbus.String('from_repo_id'): dbus.String('', variant_level=1),
                        dbus.String('reason'): dbus.String('External User', variant_level=1),
                    }, signature=dbus.Signature('sv'))),
                    signature=None),
                dbus.Struct((
                    dbus.String('Package'),         # object type
                    dbus.String('Replaced'),        # action
                    dbus.String('External User'),   # reason
                    dbus.Dictionary({               # transaction item attrs
                    }, signature=dbus.Signature('sv')),
                    dbus.Dictionary({               # package
                        dbus.String('full_nevra'): dbus.String('one-0:1-1.noarch', variant_level=1),
                        dbus.String('evr'): dbus.String('1-1', variant_level=1),
                        dbus.String('name'): dbus.String('one', variant_level=1),
                        dbus.String('epoch'): dbus.String('0', variant_level=1),
                        dbus.String('version'): dbus.String('1', variant_level=1),
                        dbus.String('release'): dbus.String('1', variant_level=1),
                        dbus.String('arch'): dbus.String('noarch', variant_level=1),
                        dbus.String('install_size'): dbus.UInt64(0, variant_level=1),
                        dbus.String('repo_id'): dbus.String('@System', variant_level=1),
                        dbus.String('from_repo_id'): dbus.String('<unknown>', variant_level=1),
                        dbus.String('reason'): dbus.String('External User', variant_level=1),
                    }, signature=dbus.Signature('sv'))),
                    signature=None)
            ], signature=dbus.Signature('(ua{sv})'))
        )

        self.iface_goal.do_transaction(dbus.Dictionary({}, signature='sv'))

    def test_distr_sync_fromrepo(self):
        '''
        attempt to distro-sync/upgrade package from repo that does not contain it returns
        empty transaction
        '''
        self.iface_rpm.upgrade(['one'], dbus.Dictionary(
            {'repo_ids': ['rpm-repo2']}, signature='sv'))
        resolved, result = self.iface_goal.resolve(
            dbus.Dictionary({}, signature='sv'))

        self.assertEqual(result, 2)
        self.assertEqual(
            resolved,
            dbus.Array([
            ],  signature=dbus.Signature('(ua{sv})'))
        )

        errors = self.iface_goal.get_transaction_problems_string()
        self.assertEqual(errors, dbus.Array([
            dbus.String("No match for argument 'one' in repositories 'rpm-repo2'")],
            signature=dbus.Signature('s')))
