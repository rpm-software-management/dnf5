# Copyright Contributors to the libdnf project.
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


class Plugin(libdnf5.plugin.IPlugin):
    def __init__(self, data):
        super(Plugin, self).__init__(data)
        self.base = self.get_base()

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
                      'description': 'Libdnf5 plugin written in Python. Processes the LOAD_CONFIG_FROM_FILE hook '
                                     'and writes the current value of the "skip_if_unavailable" option to the log.'}
        return attributes.get(name, None)

    def repos_configured(self):
        logger = self.base.get_logger()
        config = self.base.get_config()
        logger.info(self.get_name() + ' - skip_if_unavailable = ' +
                    str(config.skip_if_unavailable))
        print(self.get_name() + ' - skip_if_unavailable = ' +
              str(config.skip_if_unavailable))
        return True

    def finish(self):
        pass
