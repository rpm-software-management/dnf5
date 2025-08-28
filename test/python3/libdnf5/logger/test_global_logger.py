# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later

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
