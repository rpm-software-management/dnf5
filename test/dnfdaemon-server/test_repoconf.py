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

IFACE_REPOCONF = '{}.rpm.RepoConf'.format(support.DNFDAEMON_BUS_NAME)

REPO_TEMPLATE = '''[{reponame}]
name=Repository {reponame}
baseurl=http://example.com/{reponame}
enabled=1
'''


def configure_repo(config_file, reponame):
    os.makedirs(os.path.dirname(config_file), exist_ok=True)
    with open(config_file, 'a') as f:
        f.write(REPO_TEMPLATE.format(reponame=reponame))


class RepoConfTest(unittest.TestCase):

    def setUp(self):
        # prepare install root with repository configuration
        self.installroot = tempfile.mkdtemp(prefix="dnfdaemon-test-")
        configure_repo(os.path.join(self.installroot, 'etc/dnf/dnf.conf'), 'main_repo')
        configure_repo(os.path.join(self.installroot, 'etc/yum.repos.d/repo1.repo'), 'repo1')
        configure_repo(os.path.join(self.installroot, 'etc/yum.repos.d/repo2.repo'), 'repo2')
        # get the session for the installroot
        self.bus = dbus.SystemBus()
        self.iface_session = dbus.Interface(
            self.bus.get_object(support.DNFDAEMON_BUS_NAME, support.DNFDAEMON_OBJECT_PATH),
            dbus_interface=support.IFACE_SESSION_MANAGER)
        self.session = self.iface_session.open_session({"installroot": self.installroot})
        self.iface_repoconf = dbus.Interface(
            self.bus.get_object(support.DNFDAEMON_BUS_NAME, self.session),
            dbus_interface=IFACE_REPOCONF)
             

    def tearDown(self):
        self.iface_session.close_session(self.session)
        shutil.rmtree(self.installroot)

    def test_list_repositories(self):
        # get list of all repositories
        self.assertEqual(
            self.iface_repoconf.list({}),
            dbus.Array([
                dbus.Dictionary({
                    dbus.String('baseurl'): dbus.String('http://example.com/main_repo', variant_level=1),
                    dbus.String('enabled'): dbus.String('1', variant_level=1),
                    dbus.String('name'): dbus.String('Repository main_repo', variant_level=1),
                    dbus.String('repoid'): dbus.String('main_repo', variant_level=1)},
                    signature=dbus.Signature('sv')),
                dbus.Dictionary({
                    dbus.String('baseurl'): dbus.String('http://example.com/repo1', variant_level=1),
                    dbus.String('enabled'): dbus.String('1', variant_level=1),
                    dbus.String('name'): dbus.String('Repository repo1', variant_level=1),
                    dbus.String('repoid'): dbus.String('repo1', variant_level=1)},
                    signature=dbus.Signature('sv')),
                dbus.Dictionary({
                    dbus.String('baseurl'): dbus.String('http://example.com/repo2', variant_level=1),
                    dbus.String('enabled'): dbus.String('1', variant_level=1),
                    dbus.String('name'): dbus.String('Repository repo2', variant_level=1),
                    dbus.String('repoid'): dbus.String('repo2', variant_level=1)},
                    signature=dbus.Signature('sv'))
                ], signature=dbus.Signature('a{sv}'))
        )
        # filter several repositories
        self.assertEqual(
            self.iface_repoconf.list({'ids': ['repo1', 'repo2']}),
            dbus.Array([
                dbus.Dictionary({
                    dbus.String('baseurl'): dbus.String('http://example.com/repo1', variant_level=1),
                    dbus.String('enabled'): dbus.String('1', variant_level=1),
                    dbus.String('name'): dbus.String('Repository repo1', variant_level=1),
                    dbus.String('repoid'): dbus.String('repo1', variant_level=1)},
                    signature=dbus.Signature('sv')),
                dbus.Dictionary({
                    dbus.String('baseurl'): dbus.String('http://example.com/repo2', variant_level=1),
                    dbus.String('enabled'): dbus.String('1', variant_level=1),
                    dbus.String('name'): dbus.String('Repository repo2', variant_level=1),
                    dbus.String('repoid'): dbus.String('repo2', variant_level=1)},
                    signature=dbus.Signature('sv'))
                ], signature=dbus.Signature('a{sv}'))
        )
        # filter non-existent repositories
        self.assertEqual(
            self.iface_repoconf.list({'ids': ['nonrepo-1', 'nonrepo-2']}),
            dbus.Array([
                ], signature=dbus.Signature('a{sv}')))

    def test_get_repository(self):
        self.assertEqual(
            self.iface_repoconf.get('main_repo'),
            dbus.Dictionary({
                dbus.String('baseurl'): dbus.String('http://example.com/main_repo', variant_level=1),
                dbus.String('enabled'): dbus.String('1', variant_level=1),
                dbus.String('name'): dbus.String('Repository main_repo', variant_level=1),
                dbus.String('repoid'): dbus.String('main_repo', variant_level=1)},
                signature=dbus.Signature('sv'))
        )

    def test_enable_disable(self):
        # try to disable non-existent repo
        self.assertEqual(
            self.iface_repoconf.disable(['nonrepo-1']),
            dbus.Array([], signature=dbus.Signature('s'))
        )
        # disable repo
        self.assertEqual(
            self.iface_repoconf.disable(['main_repo']),
            dbus.Array([dbus.String('main_repo')], signature=dbus.Signature('s'))
        )
        self.assertEqual(
            self.iface_repoconf.get('main_repo'),
            dbus.Dictionary({
                dbus.String('baseurl'): dbus.String('http://example.com/main_repo', variant_level=1),
                dbus.String('enabled'): dbus.String('0', variant_level=1),
                dbus.String('name'): dbus.String('Repository main_repo', variant_level=1),
                dbus.String('repoid'): dbus.String('main_repo', variant_level=1)},
                signature=dbus.Signature('sv'))
        )
        # enable repo
        self.assertEqual(
            self.iface_repoconf.enable(['main_repo']),
            dbus.Array([dbus.String('main_repo')], signature=dbus.Signature('s'))
        )
        self.assertEqual(
            self.iface_repoconf.get('main_repo'),
            dbus.Dictionary({
                dbus.String('baseurl'): dbus.String('http://example.com/main_repo', variant_level=1),
                dbus.String('enabled'): dbus.String('1', variant_level=1),
                dbus.String('name'): dbus.String('Repository main_repo', variant_level=1),
                dbus.String('repoid'): dbus.String('main_repo', variant_level=1)},
                signature=dbus.Signature('sv'))
        )
