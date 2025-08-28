# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later

import dbus
import os
import subprocess

import support


class RemoveTest(support.InstallrootCase):

    def setUp(self):
        super(RemoveTest, self).setUp()
        # install a package inside the installroot
        pkg_file = os.path.join(
            support.PROJECT_BINARY_DIR, "test/data/repos-rpm/rpm-repo1/one-1-1.noarch.rpm")
        res = subprocess.run(
            ["rpm", "--root", self.installroot, "-U", pkg_file])
        self.assertEqual(
            res.returncode, 0, "Installation of test package '{}' failed.".format(pkg_file))

    def test_remove_package(self):
        # remove an installed package
        self.iface_rpm.remove(['one'], dbus.Dictionary({}, signature='sv'))

        resolved, result = self.iface_goal.resolve(
            dbus.Dictionary({}, signature='sv'))
        self.sanitize_transaction(resolved)

        self.assertEqual(result, 0)
        self.assertCountEqual(
            resolved,
            dbus.Array([
                dbus.Struct((
                    dbus.String('Package'),         # object type
                    dbus.String('Remove'),          # action
                    dbus.String('User'),            # reason
                    dbus.Dictionary({               # transaction item attrs
                    }, signature=dbus.Signature('sv')),
                    dbus.Dictionary({               # package
                        dbus.String('full_nevra'): dbus.String('one-0:1-1.noarch', variant_level=1),
                        dbus.String('arch'): dbus.String('noarch', variant_level=1),
                        dbus.String('epoch'): dbus.String('0', variant_level=1),
                        dbus.String('evr'): dbus.String('1-1', variant_level=1),
                        dbus.String('name'): dbus.String('one', variant_level=1),
                        dbus.String('install_size'): dbus.UInt64(0, variant_level=1),
                        dbus.String('release'): dbus.String('1', variant_level=1),
                        dbus.String('repo_id'): dbus.String('@System', variant_level=1),
                        dbus.String('version'): dbus.String('1', variant_level=1),
                        dbus.String('from_repo_id'): dbus.String('<unknown>', variant_level=1),
                        dbus.String('reason'): dbus.String('External User', variant_level=1),
                    }, signature=dbus.Signature('sv'))),
                    signature=None)
            ], signature=dbus.Signature('(ua{sv})'))
        )

        self.iface_goal.do_transaction(dbus.Dictionary({}, signature='sv'))
