# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later
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

import libdnf5

import base_test_case


class TestGlobalLogger(base_test_case.BaseTestCase):
    def test_creating_global_logger_again(self):
        # Try creating two instances of global logger
        router = libdnf5.logger.LogRouter()
        global_logger = libdnf5.logger.GlobalLogger()
        global_logger.set(router, libdnf5.logger.Logger.Level_DEBUG)
        with self.assertRaisesRegex(libdnf5.exception.UserAssertionError,
                                    'libdnf5/logger/global_logger.cpp:[0-9]+: libdnf5::GlobalLogger::GlobalLogger\\(\\):'
                                    ' API Assertion \'librepo_logger == nullptr\' failed:'
                                    ' Only one GlobalLogger can exist at a time'):
            libdnf5.logger.GlobalLogger()
