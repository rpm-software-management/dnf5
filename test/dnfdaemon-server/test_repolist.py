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
import shutil
import tempfile
import unittest

import support

class RepoTest(unittest.TestCase):

    def setUp(self):
        self.installroot = tempfile.mkdtemp(prefix="dnfdaemon-test-")
        reposdir = os.path.join(self.installroot, 'etc/yum.repos.d')

        # generate .repo files
        support.create_reposdir(reposdir)

        self.bus = dbus.SystemBus()
        self.iface_session = dbus.Interface(
            self.bus.get_object(support.DNFDAEMON_BUS_NAME, support.DNFDAEMON_OBJECT_PATH),
            dbus_interface=support.IFACE_SESSION_MANAGER)
        self.session = self.iface_session.open_session(dict(reposdir=reposdir))
        self.iface_repo = dbus.Interface(
            self.bus.get_object(support.DNFDAEMON_BUS_NAME, self.session),
            dbus_interface=support.IFACE_REPO)

    def tearDown(self):
        self.iface_session.close_session(self.session)
        shutil.rmtree(self.installroot)

    def test_list_repos(self):
        # get list of all repositories
        self.assertEqual(
            self.iface_repo.list({}),
            dbus.Array([
                dbus.Dictionary({
                    dbus.String('name'): dbus.String('Repository repo-a', variant_level=1),
                    dbus.String('enabled'): dbus.Boolean(True, variant_level=1),
                    dbus.String('id'): dbus.String('repo-a', variant_level=1)},
                    signature=dbus.Signature('sv')),
                dbus.Dictionary({
                    dbus.String('name'): dbus.String('Repository repo-b', variant_level=1),
                    dbus.String('enabled'): dbus.Boolean(True, variant_level=1),
                    dbus.String('id'): dbus.String('repo-b', variant_level=1)},
                    signature=dbus.Signature('sv')),
                ],
                signature=dbus.Signature('a{sv}'))
        )


    def test_list_repos_spec(self):
        # get list of specified repositories
        self.assertEqual(
            self.iface_repo.list({"patterns": ['repo-a']}),
            dbus.Array([
                dbus.Dictionary({
                    dbus.String('name'): dbus.String('Repository repo-a', variant_level=1),
                    dbus.String('enabled'): dbus.Boolean(True, variant_level=1),
                    dbus.String('id'): dbus.String('repo-a', variant_level=1)},
                    signature=dbus.Signature('sv')),
                ],
                signature=dbus.Signature('a{sv}'))
        )
