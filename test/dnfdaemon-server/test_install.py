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

        self.assertResolveErrorsEmpty(errors)

        self.iface_goal.do_transaction(dbus.Dictionary({}, signature='sv'))

    def test_install_strict(self):
        '''with strict=True attempt to install nonexistent package returns error'''
        self.iface_rpm.install(['no_one'], dbus.Dictionary({'strict': True}, signature='sv'))
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
                        dbus.String('action'): dbus.UInt32(0, variant_level=1),
                        dbus.String('problem'): dbus.UInt32(4, variant_level=1),
                        dbus.String('goal_job_settings'): dbus.Dictionary({
                            dbus.String('to_repo_ids'): dbus.Array([
                                ], signature=dbus.Signature('s'), variant_level=1)
                            }, signature=dbus.Signature('sv'), variant_level=1),
                        dbus.String('report'): dbus.String('no_one', variant_level=1),
                        dbus.String('report_list'): dbus.Array([
                            ], signature=dbus.Signature('s'), variant_level=1)
                        }, signature=dbus.Signature('sv'))
                    ], signature=dbus.Signature('a{sv}'), variant_level=1),
                }, signature=dbus.Signature('sv'), variant_level=1)
        )

    def test_install_nostrict(self):
        '''
        with strict=True attempt to install nonexistent package returns empty
        transaction
        '''
        self.iface_rpm.install(['no_one'], dbus.Dictionary({'strict': False}, signature='sv'))

        resolved, errors = self.iface_goal.resolve(dbus.Dictionary({}, signature='sv'))
        self.assertCountEqual(
            resolved,
            dbus.Array([
                ], signature=dbus.Signature('(ua{sv})'))
            )

        self.assertDictEqual(
            errors,
            dbus.Dictionary({
                dbus.String('transaction_problems'): dbus.UInt32(0, variant_level=1),
                dbus.String('transaction_solver_problems'): dbus.String('', variant_level=1),
                dbus.String('goal_problems'): dbus.Array([
                    dbus.Dictionary({
                        dbus.String('action'): dbus.UInt32(0, variant_level=1),
                        dbus.String('problem'): dbus.UInt32(4, variant_level=1),
                        dbus.String('goal_job_settings'): dbus.Dictionary({
                            dbus.String('to_repo_ids'): dbus.Array([
                                ], signature=dbus.Signature('s'), variant_level=1)
                            }, signature=dbus.Signature('sv'), variant_level=1),
                        dbus.String('report'): dbus.String('no_one', variant_level=1),
                        dbus.String('report_list'): dbus.Array([
                            ], signature=dbus.Signature('s'), variant_level=1)
                        }, signature=dbus.Signature('sv'))
                    ], signature=dbus.Signature('a{sv}'), variant_level=1),
                }, signature=dbus.Signature('sv'), variant_level=1)
        )

    def test_install_fromrepo(self):
        '''
        attempt to install package from repo that does not contain it return error
        '''
        self.iface_rpm.install(['one'], dbus.Dictionary({'repo_ids': ['rpm-repo2']}, signature='sv'))
        resolved, errors = self.iface_goal.resolve(dbus.Dictionary({}, signature='sv'))

        self.assertCountEqual(
            resolved,
            dbus.Array([
                ], signature=dbus.Signature("(ua{sv})")),
            )

        self.assertDictEqual(
            errors,
            dbus.Dictionary({
                dbus.String('transaction_problems'): dbus.UInt32(32, variant_level=1),
                dbus.String('transaction_solver_problems'): dbus.String('', variant_level=1),
                dbus.String('goal_problems'): dbus.Array([
                    dbus.Dictionary({
                        dbus.String('action'): dbus.UInt32(0, variant_level=1),
                        dbus.String('problem'): dbus.UInt32(32, variant_level=1),
                        dbus.String('goal_job_settings'): dbus.Dictionary({
                            dbus.String('to_repo_ids'): dbus.Array([
                                dbus.String('rpm-repo2', variant_level=1)
                                ], signature=dbus.Signature('s'), variant_level=1)
                            }, signature=dbus.Signature('sv'), variant_level=1),
                        dbus.String('report'): dbus.String('one', variant_level=1),
                        dbus.String('report_list'): dbus.Array([
                            ], signature=dbus.Signature('s'), variant_level=1)
                        }, signature=dbus.Signature('sv'))
                    ], signature=dbus.Signature('a{sv}'), variant_level=1),
                }, signature=dbus.Signature('sv'), variant_level=1)
        )
