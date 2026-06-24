# Copyright Contributors to the DNF5 project.
# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: LGPL-2.1-or-later
#
# This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
#
# Libdnf is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2.1 of the License, or
# (at your option) any later version.
#
# Libdnf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

import libdnf5


# Inherit from IPlugin2_1 to get all hooks including `goal_resolved`.
# You can use IPlugin instead, if `goal_resolved` is not needed.
# See https://dnf5.readthedocs.io/en/latest/api/python/libdnf5_plugin.html
class Plugin(libdnf5.plugin.IPlugin2_1):
    def __init__(self, data):
        super(Plugin, self).__init__(data)
        self.base = self.get_base()

    # --- IPlugin: required metadata methods ---

    @staticmethod
    def get_api_version():
        return libdnf5.PluginAPIVersion(2, 1)

    @staticmethod
    def get_name():
        return 'plugin'

    @staticmethod
    def get_version():
        return libdnf5.plugin.Version(0, 1, 0)

    def get_attribute(self, name):
        attributes = {'author_name': 'Jaroslav Rohel',
                      'author_email': 'jrohel@redhat.com',
                      'description': 'Libdnf5 plugin written in Python. Implements all available hooks.'}
        return attributes.get(name, None)

    # --- IPlugin: optional hooks ---

    def init(self):
        logger = self.base.get_logger()
        logger.info(self.get_name() + ' - init')

    def pre_base_setup(self):
        logger = self.base.get_logger()
        logger.info(self.get_name() + ' - pre_base_setup')

    def post_base_setup(self):
        logger = self.base.get_logger()
        logger.info(self.get_name() + ' - post_base_setup')

    def repos_configured(self):
        logger = self.base.get_logger()
        config = self.base.get_config()
        logger.info(self.get_name() + ' - repos_configured, skip_if_unavailable = ' +
                    str(config.skip_if_unavailable))

    def repos_loaded(self):
        logger = self.base.get_logger()
        logger.info(self.get_name() + ' - repos_loaded')

    def pre_add_cmdline_packages(self, paths):
        logger = self.base.get_logger()
        logger.info(self.get_name() + ' - pre_add_cmdline_packages: ' +
                    str(paths))

    def post_add_cmdline_packages(self):
        logger = self.base.get_logger()
        logger.info(self.get_name() + ' - post_add_cmdline_packages')

    def pre_transaction(self, transaction):
        logger = self.base.get_logger()
        logger.info(self.get_name() + ' - pre_transaction: ' +
                    str(transaction.get_transaction_packages_count()) +
                    ' packages in transaction')

    def post_transaction(self, transaction):
        logger = self.base.get_logger()
        logger.info(self.get_name() + ' - post_transaction: ' +
                    str(transaction.get_transaction_packages_count()) +
                    ' packages in transaction')

    def finish(self):
        pass

    # --- IPlugin2_1: additional hooks (requires API version 2.1) ---

    def goal_resolved(self, transaction):
        logger = self.base.get_logger()
        logger.info(self.get_name() + ' - goal_resolved: ' +
                    str(transaction.get_transaction_packages_count()) +
                    ' packages in transaction')
