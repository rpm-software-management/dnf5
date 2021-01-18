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

import os

DNFDAEMON_BUS_NAME = 'org.rpm.dnf.v0'
DNFDAEMON_OBJECT_PATH = '/' + DNFDAEMON_BUS_NAME.replace('.', '/')

IFACE_SESSION_MANAGER = '{}.SessionManager'.format(DNFDAEMON_BUS_NAME)
IFACE_REPO = '{}.rpm.Repo'.format(DNFDAEMON_BUS_NAME)
IFACE_RPM = '{}.rpm.Rpm'.format(DNFDAEMON_BUS_NAME)


def create_reposdir(reposdir):
    REPO_TEMPLATE = '''[{reponame}]
name=Repository {reponame}
baseurl=file:///{repopath}
enabled=1
'''
    data_path = os.path.abspath(
        os.path.join(os.path.dirname(__file__), 'test_data/repos/'))

    def configure_repo(config_file, reponame):
        repopath = os.path.join(data_path, reponame)
        os.makedirs(os.path.dirname(config_file), exist_ok=True)
        with open(config_file, 'a') as f:
            f.write(REPO_TEMPLATE.format(reponame=reponame, repopath=repopath))

    os.makedirs(reposdir, exist_ok=True)
    for repo_dir in os.scandir(data_path):
        if not repo_dir.is_dir():
            continue
        configure_repo(
            os.path.join(reposdir, '{}.repo'.format(repo_dir.name)),
            repo_dir.name)
