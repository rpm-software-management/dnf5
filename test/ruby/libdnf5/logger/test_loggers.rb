# Copyright Contributors to the DNF5 project.
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

require 'test/unit'
include Test::Unit::Assertions

require 'stringio'

require 'libdnf5/logger'

# Tests overloading of log() method
class LibdnfLoggerCB1 < Libdnf5::Logger::Logger
    def initialize(stream)
        super()
        @stream = stream
    end

    def log_line(level, message)
       @stream.write("%s: %s\n" % [Libdnf5::Logger::Logger::level_to_cstr(level), message])
    end

    def write(time, pid, level, message)
       @stream.write("Bad message\n")
    end
end

# Tests overloading of write() method
class LibdnfLoggerCB2 < Libdnf5::Logger::Logger
    def initialize(stream)
        super()
        @stream = stream
    end

    def write(time, pid, level, message)
       @stream.write("%s: %s\n" % [Libdnf5::Logger::Logger::level_to_cstr(level), message])
    end
end

class TestLoggers < Test::Unit::TestCase
    def test_logger()
        expected_logger_content = "CRITICAL: Critical message\n" \
                                  "ERROR: Error message\n"       \
                                  "WARNING: Warning message\n"   \
                                  "NOTICE: Notice message\n"     \
                                  "INFO: Info message\n"         \
                                  "DEBUG: Debug message\n"       \
                                  "TRACE: Trace message\n"

        stream1 = StringIO.new()
        logger1 = LibdnfLoggerCB1.new(stream1)
        stream2 = StringIO.new()
        logger2 = LibdnfLoggerCB2.new(stream2)
        for logger in [logger1, logger2]
            logger.critical('Critical message')
            logger.error('Error message')
            logger.warning('Warning message')
            logger.notice('Notice message')
            logger.info('Info message')
            logger.debug('Debug message')
            logger.trace('Trace message')
        end
        content1 = stream1.string()
        assert_equal(content1, expected_logger_content)
        content2 = stream2.string()
        assert_equal(content2, expected_logger_content)
    end

    def test_logrouter()
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
        log_router = Libdnf5::Logger::LogRouter.new()
        # Create circular memory buffer logger with capacity 10 messages (4 pre-allocated from start).
        max_items_to_keep = 10
        reserve = 4
        memory_buffer_logger = Libdnf5::Logger::MemoryBufferLogger.new(max_items_to_keep, reserve)
        logger_uniq_ptr = Libdnf5::Logger::LoggerUniquePtr.new(memory_buffer_logger)
        log_router.add_logger(logger_uniq_ptr)

        # Test the number of registered loggers.
        assert_equal(log_router.get_loggers_count(), 1)

        # ====================
        # 2. Write messages into log_router. They will be routed into memory_buffer_logger.
        # ====================
        for i in 0..1
            log_router.log(Libdnf5::Logger::Logger::Level_CRITICAL, "Critical message")
            log_router.log(Libdnf5::Logger::Logger::Level_ERROR, "Error message")
            log_router.log(Libdnf5::Logger::Logger::Level_WARNING, "Warning message")
            log_router.log(Libdnf5::Logger::Logger::Level_NOTICE, "Notice message")
            log_router.log(Libdnf5::Logger::Logger::Level_INFO, "Info message")
            log_router.log(Libdnf5::Logger::Logger::Level_DEBUG, "Debug message")
            log_router.log(Libdnf5::Logger::Logger::Level_TRACE, "Trace message")
        end

        # ====================
        # 3. Create two StreamLogger instances which replace (swap) MemoryBufferLogger instance in log_router.
        # ====================
        # Create two streams for logging. Save pointers to them. Pointers will be needed later
        # in test of streams content.
        stream1 = StringIO.new()
        stream2 = StringIO.new()

        # Create StreamLogger instance and swap it with MemoryBufferLogger instance which was added
        # into LogRouter before.
        tmp_logger = LibdnfLoggerCB2.new(stream1)
        tmp_logger_uniq_ptr = Libdnf5::Logger::LoggerUniquePtr.new(tmp_logger)
        # In the log_router is registered only instance of MemoryBufferLogger just now.
        # The index of the first logger is "0".
        log_router.swap_logger(tmp_logger_uniq_ptr, 0)

        # Create secondary StreamLogger instance and add it to LogRouter as another logger.
        log2_stream = LibdnfLoggerCB2.new(stream2)
        log2_stream_uniq_ptr = Libdnf5::Logger::LoggerUniquePtr.new(log2_stream)
        log_router.add_logger(log2_stream_uniq_ptr)

        # Test the number of registered loggers.
        assert_equal(log_router.get_loggers_count(), 2)

        # ====================
        # 4. Write messages from original (replaced) MemoryBufferLoger instance into LogRouter instance.
        #    They will be routed to both attached stream loggers.
        # ====================
        memory_buffer_logger.write_to_logger(log_router)

        # ====================
        # 5. Write additional message into LogRouter instance.
        # ====================
        log_router.log(Libdnf5::Logger::Logger::Level_INFO, "Info additional message")

        # ====================
        # 6. Check content of streams of both StreamLogger instances.
        # ====================
        content = stream1.string()
        assert_equal(content, expected_logger_content)
        content = stream2.string()
        assert_equal(content, expected_logger_content)
    end
end
