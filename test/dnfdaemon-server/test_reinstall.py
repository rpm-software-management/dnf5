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
import os
import subprocess

import support

class ReinstallTest(support.InstallrootCase):

    def setUp(self):
        super(ReinstallTest, self).setUp()
        # install a package inside the installroot
        pkg_file = os.path.join(support.PROJECT_BINARY_DIR, "test/data/repos-rpm/rpm-repo1/one-1-1.noarch.rpm")
        res = subprocess.run(["rpm", "--root", self.installroot, "-U", pkg_file])
        self.assertEqual(res.returncode, 0, "Installation of test package '{}' failed.".format(pkg_file))

    def test_reinstall_package(self):
        # reinstall an installed package
        self.iface_rpm.reinstall(['one'], dbus.Dictionary({}, signature='sv'))

        resolved, errors = self.iface_goal.resolve(dbus.Dictionary({}, signature='sv'))

        # id of package depends on order of the repos in the sack which varies
        # between runs so we can't rely on the value
        for action, pkg in resolved:
            pkg.pop('id')
            pkg.pop('package_size')

        self.assertCountEqual(
            resolved,
            dbus.Array([
                dbus.Struct((
                    dbus.UInt32(9),   # action reinstall
                    dbus.Dictionary({ # package
                        dbus.String('evr'): dbus.String('1-1', variant_level=1),
                        dbus.String('name'): dbus.String('one', variant_level=1),
                        dbus.String('epoch'): dbus.String('0', variant_level=1),
                        dbus.String('version'): dbus.String('1', variant_level=1),
                        dbus.String('release'): dbus.String('1', variant_level=1),
                        dbus.String('arch'): dbus.String('noarch', variant_level=1),
                        dbus.String('install_size'): dbus.UInt64(0, variant_level=1),
                        dbus.String('repo'): dbus.String('rpm-repo1', variant_level=1),
                        }, signature=dbus.Signature('sv'))),
                    signature=None),
                dbus.Struct((
                    dbus.UInt32(10),  # action reinstalled
                    dbus.Dictionary({ # package
                        dbus.String('evr'): dbus.String('1-1', variant_level=1),
                        dbus.String('name'): dbus.String('one', variant_level=1),
                        dbus.String('epoch'): dbus.String('0', variant_level=1),
                        dbus.String('version'): dbus.String('1', variant_level=1),
                        dbus.String('release'): dbus.String('1', variant_level=1),
                        dbus.String('arch'): dbus.String('noarch', variant_level=1),
                        dbus.String('install_size'): dbus.UInt64(0, variant_level=1),
                        dbus.String('repo'): dbus.String('@System', variant_level=1),
                        }, signature=dbus.Signature('sv'))),
                    signature=None)
                ], signature=dbus.Signature('(ua{sv})')),
        )

        self.assertResolveErrorsEmpty(errors)

        self.iface_goal.do_transaction(dbus.Dictionary({}, signature='sv'))

    def test_reinstall_fromrepo(self):
        '''
        attempt to reinstall package from repo that does not contain it returns
        empty transaction
        '''
        self.iface_rpm.reinstall(['one'], dbus.Dictionary({'repo_ids': ['rpm-repo2']}, signature='sv'))
        resolved, errors = self.iface_goal.resolve(dbus.Dictionary({}, signature='sv'))
        self.assertCountEqual(
            resolved,
            dbus.Array([
                ], signature=dbus.Signature('(ua{sv})'))
            )

        self.assertDictEqual(
            errors,
            dbus.Dictionary({
                dbus.String('transaction_problems'): dbus.UInt32(32, variant_level=1),
                dbus.String('transaction_solver_problems'): dbus.String('', variant_level=1),
                dbus.String('goal_problems'): dbus.Array([
                    dbus.Dictionary({
                        dbus.String('action'): dbus.UInt32(2, variant_level=1),
                        dbus.String('problem'): dbus.UInt32(32, variant_level=1),
                        dbus.String('goal_job_settings'): dbus.Dictionary({
                            dbus.String('to_repo_ids'): dbus.Array([
                                dbus.String('rpm-repo2')
                                ], signature=dbus.Signature('s'), variant_level=1)
                            }, signature=dbus.Signature('sv'), variant_level=1),
                        dbus.String('report'): dbus.String('one', variant_level=1),
                        dbus.String('report_list'): dbus.Array([
                            ], signature=dbus.Signature('s'), variant_level=1)
                        }, signature=dbus.Signature('sv'))
                    ], signature=dbus.Signature('a{sv}'), variant_level=1),
                }, signature=dbus.Signature('sv'), variant_level=1)
        )

    def test_reinstall_not_installed(self):
        '''
        attempt to reinstall package available but not installed returns empty
        transaction
        '''
        self.iface_rpm.reinstall(['two'], dbus.Dictionary({'repo_ids': ['rpm-repo2']}, signature='sv'))
        resolved, errors = self.iface_goal.resolve(dbus.Dictionary({}, signature='sv'))
        self.assertCountEqual(
            resolved,
            dbus.Array([
                ], signature=dbus.Signature('(ua{sv})'))
            )


        self.assertDictEqual(
            errors,
            dbus.Dictionary({
                dbus.String('transaction_problems'): dbus.UInt32(64, variant_level=1),
                dbus.String('transaction_solver_problems'): dbus.String('', variant_level=1),
                dbus.String('goal_problems'): dbus.Array([
                    dbus.Dictionary({
                        dbus.String('action'): dbus.UInt32(2, variant_level=1),
                        dbus.String('problem'): dbus.UInt32(64, variant_level=1),
                        dbus.String('goal_job_settings'): dbus.Dictionary({
                            dbus.String('to_repo_ids'): dbus.Array([
                                dbus.String('rpm-repo2')
                                ], signature=dbus.Signature('s'), variant_level=1)
                            }, signature=dbus.Signature('sv'), variant_level=1),
                        dbus.String('report'): dbus.String('two', variant_level=1),
                        dbus.String('report_list'): dbus.Array([
                            ], signature=dbus.Signature('s'), variant_level=1)
                        }, signature=dbus.Signature('sv'))
                    ], signature=dbus.Signature('a{sv}'), variant_level=1),
                }, signature=dbus.Signature('sv'), variant_level=1)
        )

    def test_reinstall_non_existent_fromrepo(self):
        '''
        attempt to reinstall package from repo that does not exist it returns
        empty transaction
        '''
        self.iface_rpm.reinstall(['three'], dbus.Dictionary({}, signature='sv'))
        resolved, errors = self.iface_goal.resolve(dbus.Dictionary({}, signature='sv'))
        self.assertCountEqual(
            resolved,
            dbus.Array([
                ], signature=dbus.Signature('(ua{sv})'))
            )

        self.assertDictEqual(
            errors,
            dbus.Dictionary({
                dbus.String('transaction_problems'): dbus.UInt32(4, variant_level=1),
                dbus.String('transaction_solver_problems'): dbus.String('', variant_level=1),
                dbus.String('goal_problems'): dbus.Array([
                    dbus.Dictionary({
                        dbus.String('action'): dbus.UInt32(2, variant_level=1),
                        dbus.String('problem'): dbus.UInt32(4, variant_level=1),
                        dbus.String('goal_job_settings'): dbus.Dictionary({
                            dbus.String('to_repo_ids'): dbus.Array([
                                ], signature=dbus.Signature('s'), variant_level=1)
                            }, signature=dbus.Signature('sv'), variant_level=1),
                        dbus.String('report'): dbus.String('three', variant_level=1),
                        dbus.String('report_list'): dbus.Array([
                            ], signature=dbus.Signature('s'), variant_level=1)
                        }, signature=dbus.Signature('sv'))
                    ], signature=dbus.Signature('a{sv}'), variant_level=1),
                }, signature=dbus.Signature('sv'), variant_level=1)
        )
