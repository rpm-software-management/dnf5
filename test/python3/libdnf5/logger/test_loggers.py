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

import io
import unittest

import libdnf5.logger


# Tests overloading of log() method
class libdnf5LoggerCB1(libdnf5.logger.Logger):
    def __init__(self, stream):
        super(libdnf5LoggerCB1, self).__init__()
        self._stream = stream

    def log_line(self, level, message):
        self._stream.write("{}: {}\n".format(
            self.level_to_cstr(level), message))

    def write(self, time, pid, level, message):
        self._stream.write("Bad message\n")


# Tests overloading of write() method
class libdnf5LoggerCB2(libdnf5.logger.Logger):
    def __init__(self, stream):
        super(libdnf5LoggerCB2, self).__init__()
        self._stream = stream

    def write(self, time, pid, level, message):
        self._stream.write("{}: {}\n".format(
            self.level_to_cstr(level), message))


class TestLoggers(unittest.TestCase):
    def test_logger(self):
        expected_logger_content = "CRITICAL: Critical message\n" \
                                  "ERROR: Error message\n"       \
                                  "WARNING: Warning message\n"   \
                                  "NOTICE: Notice message\n"     \
                                  "INFO: Info message\n"         \
                                  "DEBUG: Debug message\n"       \
                                  "TRACE: Trace message\n"

        stream1 = io.StringIO()
        logger1 = libdnf5LoggerCB1(stream1)
        stream2 = io.StringIO()
        logger2 = libdnf5LoggerCB2(stream2)
        for logger in logger1, logger2:
            logger.critical("Critical message")
            logger.error("Error message")
            logger.warning("Warning message")
            logger.notice("Notice message")
            logger.info("Info message")
            logger.debug("Debug message")
            logger.trace("Trace message")
        content1 = stream1.getvalue()
        self.assertEqual(content1, expected_logger_content)
        content2 = stream2.getvalue()
        self.assertEqual(content2, expected_logger_content)

    def test_logrouter(self):
        expected_logger_content = "INFO: Info message\n"         \
                                  "DEBUG: Debug message\n"       \
                                  "TRACE: Trace message\n"       \
                                  "CRITICAL: Critical message\n" \
                                  "ERROR: Error message\n"       \
                                  "WARNING: Warning message\n"   \
                                  "NOTICE: Notice message\n"     \
                                  "INFO: Info message\n"         \
                                  "DEBUG: Debug message\n"       \
                                  "TRACE: Trace message\n"       \
                                  "INFO: Info additional message\n"

        # ====================
        #  1. Create a LogRouter instance with one MemoryBufferLogger instances attached.
        #  ====================
        # Create log router.
        log_router = libdnf5.logger.LogRouter()
        # Create circular memory buffer logger with capacity 10 messages (4 pre-allocated from start).
        max_items_to_keep = 10
        reserve = 4
        memory_buffer_logger = libdnf5.logger.MemoryBufferLogger(
            max_items_to_keep, reserve)
        logger_uniq_ptr = libdnf5.logger.LoggerUniquePtr(memory_buffer_logger)
        log_router.add_logger(logger_uniq_ptr)

        # Test the number of registered loggers.
        self.assertEqual(log_router.get_loggers_count(), 1)

        # ====================
        # 2. Write messages into log_router. They will be routed into memory_buffer_logger.
        # ====================
        for i in range(2):
            log_router.log(libdnf5.logger.Logger.Level_CRITICAL,
                           "Critical message")
            log_router.log(libdnf5.logger.Logger.Level_ERROR, "Error message")
            log_router.log(libdnf5.logger.Logger.Level_WARNING,
                           "Warning message")
            log_router.log(libdnf5.logger.Logger.Level_NOTICE,
                           "Notice message")
            log_router.log(libdnf5.logger.Logger.Level_INFO, "Info message")
            log_router.log(libdnf5.logger.Logger.Level_DEBUG, "Debug message")
            log_router.log(libdnf5.logger.Logger.Level_TRACE, "Trace message")

        # ====================
        # 3. Create two StreamLogger instances which replace (swap) MemoryBufferLogger instance in log_router.
        # ====================
        # Create two streams for logging. Save pointers to them. Pointers will be needed later
        # in test of streams content.
        stream1 = io.StringIO()
        stream2 = io.StringIO()

        # Create StreamLogger instance and swap it with MemoryBufferLogger instance which was added
        # into LogRouter before.
        tmp_logger = libdnf5LoggerCB2(stream1)
        tmp_logger_uniq_ptr = libdnf5.logger.LoggerUniquePtr(tmp_logger)
        # In the log_router is registered only instance of MemoryBufferLogger just now.
        # The index of the first logger is "0".
        log_router.swap_logger(tmp_logger_uniq_ptr, 0)

        # Create secondary StreamLogger instance and add it to LogRouter as another logger.
        log2_stream = libdnf5LoggerCB2(stream2)
        log2_stream_uniq_ptr = libdnf5.logger.LoggerUniquePtr(log2_stream)
        log_router.add_logger(log2_stream_uniq_ptr)

        # Test the number of registered loggers.
        self.assertEqual(log_router.get_loggers_count(), 2)

        # ====================
        # 4. Write messages from original (replaced) MemoryBufferLoger instance into LogRouter instance.
        #    They will be routed to both attached stream loggers.
        # ====================
        memory_buffer_logger.write_to_logger(log_router)

        # ====================
        # 5. Write additional message into LogRouter instance.
        # ====================
        log_router.log(libdnf5.logger.Logger.Level_INFO,
                       "Info additional message")

        # ====================
        # 6. Check content of streams of both StreamLogger instances.
        # ====================
        content = stream1.getvalue()
        self.assertEqual(content, expected_logger_content)
        content = stream2.getvalue()
        self.assertEqual(content, expected_logger_content)
