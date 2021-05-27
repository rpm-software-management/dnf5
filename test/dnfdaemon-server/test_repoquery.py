# Copyright (C) 2020 Red Hat, Inc.
#
# This file is part of dnfdaemon-server: https://github.com/rpm-software-management/libdnf/
#
# Dnfdaemon-server is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Dnfdaemon-server is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with dnfdaemon-server.  If not, see <https://www.gnu.org/licenses/>.

import dbus
import os

import support

class RepoTest(support.InstallrootCase):

    def test_repoquery_all(self):
        # get list of all available packages
        pkglist = self.iface_rpm.list({"package_attrs": ["full_nevra", "repo"]})
        # id of package depends on order of the repos in the sack which varies
        # between runs so we can't rely on the value
        for pkg in pkglist:
            pkg.pop('id')
        # packages are ordered by the id hence assertCountEqual
        self.assertCountEqual(
            pkglist,
            dbus.Array([
                dbus.Dictionary({
                    dbus.String('full_nevra'): dbus.String('one-0:1-1.noarch', variant_level=1),
                    dbus.String('repo'): dbus.String('rpm-repo1', variant_level=1)},
                    signature=dbus.Signature('sv')),
                dbus.Dictionary({
                    dbus.String('full_nevra'): dbus.String('one-0:1-1.src', variant_level=1),
                    dbus.String('repo'): dbus.String('rpm-repo1', variant_level=1)},
                    signature=dbus.Signature('sv')),
                dbus.Dictionary({
                    dbus.String('full_nevra'): dbus.String('two-0:2-2.noarch', variant_level=1),
                    dbus.String('repo'): dbus.String('rpm-repo2', variant_level=1)},
                    signature=dbus.Signature('sv')),
                dbus.Dictionary({
                    dbus.String('full_nevra'): dbus.String('two-0:2-2.src', variant_level=1),
                    dbus.String('repo'): dbus.String('rpm-repo2', variant_level=1)},
                    signature=dbus.Signature('sv')),
                ],
                signature=dbus.Signature('a{sv}'))
        )

    def test_repoquery_match(self):
        # get list of all available packages
        pkglist = self.iface_rpm.list({
            "package_attrs": ["full_nevra", "repo"],
            "patterns":["one"]})
        # id of package depends on order of the repos in the sack which varies
        # between runs so we can't rely on the value
        for pkg in pkglist:
            pkg.pop('id')
        # packages are ordered by the id hence assertCountEqual
        self.assertCountEqual(
            pkglist,
            dbus.Array([
                dbus.Dictionary({
                    dbus.String('full_nevra'): dbus.String('one-0:1-1.noarch', variant_level=1),
                    dbus.String('repo'): dbus.String('rpm-repo1', variant_level=1)},
                    signature=dbus.Signature('sv')),
                dbus.Dictionary({
                    dbus.String('full_nevra'): dbus.String('one-0:1-1.src', variant_level=1),
                    dbus.String('repo'): dbus.String('rpm-repo1', variant_level=1)},
                    signature=dbus.Signature('sv')),
            ],
            signature=dbus.Signature('a{sv}'))
        )
