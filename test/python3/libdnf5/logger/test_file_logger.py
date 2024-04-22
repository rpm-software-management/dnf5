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

import libdnf5

import base_test_case

import os
import shutil

FILE_LOGGER_FILENAME = "dnf5.log"


class TestFileLogger(base_test_case.BaseTestCase):
    def setUp(self):
        super().setUp()
        config = self.base.get_config()
        config.logdir = os.path.join(
            config.installroot, "FileLoggerTestLogDir")
        self.full_log_path = os.path.join(config.logdir, FILE_LOGGER_FILENAME)

    def tearDown(self):
        super().tearDown()
        shutil.rmtree(os.path.dirname(self.full_log_path), ignore_errors=True)

    def test_file_logger_create_name(self):
        self.assertFalse(os.path.exists(self.full_log_path))
        _ = libdnf5.logger.create_file_logger(
            self.base, FILE_LOGGER_FILENAME)
        self.assertTrue(os.path.exists(self.full_log_path))

    def test_file_logger_add(self):
        log_router = self.base.get_logger()
        loggers_count_before = log_router.get_loggers_count()
        logger = libdnf5.logger.create_file_logger(self.base,
                                                   FILE_LOGGER_FILENAME)
        log_router.add_logger(logger)
        self.assertEqual(loggers_count_before + 1,
                         log_router.get_loggers_count())

    def test_with_global_logger(self):
        log_router = self.base.get_logger()
        global_logger = libdnf5.logger.GlobalLogger()
        global_logger.set(log_router.get(), libdnf5.logger.Logger.Level_DEBUG)
        logger = libdnf5.logger.create_file_logger(self.base,
                                                   FILE_LOGGER_FILENAME)
        log_router.add_logger(logger)

        # Run an action including librepo logging
        self.add_repo_repomd("repomd-repo1")

        with open(self.full_log_path) as logfile:
            self.assertTrue('[librepo]' in logfile.read())
