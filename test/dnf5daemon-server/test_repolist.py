# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later

import dbus
import os

import support


class RepoTest(support.InstallrootCase):

    def test_list_repos(self):
        # get list of all repositories
        self.assertEqual(
            self.iface_repo.list({"repo_attrs": ["name", "enabled"]}),
            dbus.Array([
                dbus.Dictionary({
                    dbus.String('name'): dbus.String('Repository 1', variant_level=1),
                    dbus.String('enabled'): dbus.Boolean(True, variant_level=1),
                    dbus.String('id'): dbus.String('rpm-repo1', variant_level=1)},
                    signature=dbus.Signature('sv')),
                dbus.Dictionary({
                    dbus.String('name'): dbus.String('Repository 2', variant_level=1),
                    dbus.String('enabled'): dbus.Boolean(True, variant_level=1),
                    dbus.String('id'): dbus.String('rpm-repo2', variant_level=1)},
                    signature=dbus.Signature('sv')),
            ],
                signature=dbus.Signature('a{sv}'))
        )

    def test_list_repos_spec(self):
        # get list of specified repositories
        self.assertEqual(
            self.iface_repo.list(
                {"repo_attrs": ["name", "enabled"], "patterns": ['rpm-repo1']}),
            dbus.Array([
                dbus.Dictionary({
                    dbus.String('name'): dbus.String('Repository 1', variant_level=1),
                    dbus.String('enabled'): dbus.Boolean(True, variant_level=1),
                    dbus.String('id'): dbus.String('rpm-repo1', variant_level=1)},
                    signature=dbus.Signature('sv')),
            ],
                signature=dbus.Signature('a{sv}'))
        )
