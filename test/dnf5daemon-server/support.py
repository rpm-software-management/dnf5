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
import shutil
import tempfile
import unittest

# the following settings come from cmake and are passed as ENVIRONMENT variables in CMakeLists.txt
PROJECT_BINARY_DIR = os.environ["PROJECT_BINARY_DIR"]

DNFDAEMON_BUS_NAME = 'org.rpm.dnf.v0'
DNFDAEMON_OBJECT_PATH = '/' + DNFDAEMON_BUS_NAME.replace('.', '/')

IFACE_SESSION_MANAGER = '{}.SessionManager'.format(DNFDAEMON_BUS_NAME)
IFACE_REPO = '{}.rpm.Repo'.format(DNFDAEMON_BUS_NAME)
IFACE_REPOCONF = '{}.rpm.RepoConf'.format(DNFDAEMON_BUS_NAME)
IFACE_RPM = '{}.rpm.Rpm'.format(DNFDAEMON_BUS_NAME)
IFACE_GOAL = '{}.Goal'.format(DNFDAEMON_BUS_NAME)


class InstallrootCase(unittest.TestCase):

    def sanitize_transaction(self, resolved):
        '''Prepare resolved transaction for assert'''
        for object_type, action, reason, trans_item_attrs, pkg in resolved:
            # id of package depends on order of the repos in the sack which varies
            # between runs so we can't rely on the value
            pkg.pop('id')
            # also package size differs according to the builder
            pkg.pop('download_size')
            # TODO(mblaha): calculate correct replaces value
            if "replaces" in trans_item_attrs:
                trans_item_attrs.pop("replaces")

    def path_to_repo_rpm(self, repo, rpm):
        return os.path.join(self.repository_base, repo, rpm)

    def setUp(self):
        super(InstallrootCase, self).setUp()
        self.maxDiff = None

        self.installroot = tempfile.mkdtemp(prefix="dnf5daemon-test-")
        self.reposdir = os.path.join(
            PROJECT_BINARY_DIR, "test/data/repos-rpm-conf.d")
        self.config_file_path = os.path.join(
            self.installroot, 'etc/dnf/dnf.conf')
        self.repository_base = os.path.join(
            PROJECT_BINARY_DIR, "test/data/repos-rpm")
        os.makedirs(os.path.dirname(self.config_file_path), exist_ok=True)
        with open(self.config_file_path, 'w') as f:
            f.write('')

        self.bus = dbus.SystemBus()
        self.iface_session = dbus.Interface(
            self.bus.get_object(DNFDAEMON_BUS_NAME, DNFDAEMON_OBJECT_PATH),
            dbus_interface=IFACE_SESSION_MANAGER)
        # Prevent loading plugins from host by setting "plugins" to False
        self.session = self.iface_session.open_session({
            "config": {
                "config_file_path": self.config_file_path,
                "installroot": self.installroot,
                "plugins": False,
                "cachedir": os.path.join(self.installroot, "var/cache/dnf"),
                "reposdir": self.reposdir,
            }
        })
        self.iface_repo = dbus.Interface(
            self.bus.get_object(DNFDAEMON_BUS_NAME, self.session),
            dbus_interface=IFACE_REPO)
        self.iface_rpm = dbus.Interface(
            self.bus.get_object(DNFDAEMON_BUS_NAME, self.session),
            dbus_interface=IFACE_RPM)
        self.iface_goal = dbus.Interface(
            self.bus.get_object(DNFDAEMON_BUS_NAME, self.session),
            dbus_interface=IFACE_GOAL)

    def tearDown(self):
        shutil.rmtree(self.installroot)
