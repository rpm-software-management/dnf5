/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "test_loggers.hpp"

#include "libdnf/logger/log_router.hpp"
#include "libdnf/logger/memory_buffer_logger.hpp"
#include "libdnf/logger/stream_logger.hpp"

#include <memory>
#include <sstream>

CPPUNIT_TEST_SUITE_REGISTRATION(LoggersTest);


void LoggersTest::setUp() {}


void LoggersTest::tearDown() {}


// Test of logger infrastructure:
// 1. Create a LogRouter instance with one MemoryBufferLogger instances attached.
// 2. Write messages into a LogRouter instance.
// 3. Create two StreamLogger instances which replace (swap) MemoryBufferLogger instance in log_router.
// 4. Write messages from original (replaced) MemoryBufferLoger instance into LogRouter instance.
// 5. Write aditional message into LogRouter instance.
// 6. Check content of streams of both StreamLogger instances.
void LoggersTest::test_loggers() {
    const time_t first_msg_time = 1582604702;  // Timestamp of the first message. "2020-02-25T04:25:02Z"
    const pid_t pid = 25;                      // Process identifier.

    time_t msg_time = first_msg_time;

    // Text that is expected in the logs.
    const std::string expected_loggers_content =
        "2020-02-25T04:25:06Z [25] INFO Info message\n"
        "2020-02-25T04:25:07Z [25] DEBUG Debug message\n"
        "2020-02-25T04:25:08Z [25] TRACE Trace message\n"
        "2020-02-25T04:25:09Z [25] CRITICAL Critical message\n"
        "2020-02-25T04:25:10Z [25] ERROR Error message\n"
        "2020-02-25T04:25:11Z [25] WARNING Warning message\n"
        "2020-02-25T04:25:12Z [25] NOTICE Notice message\n"
        "2020-02-25T04:25:13Z [25] INFO Info message\n"
        "2020-02-25T04:25:14Z [25] DEBUG Debug message\n"
        "2020-02-25T04:25:15Z [25] TRACE Trace message\n"
        "2020-02-25T04:25:16Z [25] INFO Info additional message\n";

    // ====================
    // 1. Create a LogRouter instance with one MemoryBufferLogger instances attached.
    // ====================
    // Create log router.
    std::unique_ptr<libdnf::LogRouter> log_router = std::make_unique<libdnf::LogRouter>();
    // Create circular memory buffer logger with capacity 10 messages (4 pre-allocated from start).
    const std::size_t max_items_to_keep = 10;
    const std::size_t reserve = 4;
    log_router->add_logger(std::make_unique<libdnf::MemoryBufferLogger>(max_items_to_keep, reserve));

    // Test the number of registered loggers.
    CPPUNIT_ASSERT_EQUAL(log_router->get_loggers_count(), static_cast<size_t>(1));

    // ====================
    // 2. Write messages into log_router. They will be routed into memory_buffer_logger.
    // ====================
    for (int i = 0; i < 2; ++i) {
        log_router->write(msg_time++, pid, libdnf::Logger::Level::CRITICAL, "Critical message");
        log_router->write(msg_time++, pid, libdnf::Logger::Level::ERROR, "Error message");
        log_router->write(msg_time++, pid, libdnf::Logger::Level::WARNING, "Warning message");
        log_router->write(msg_time++, pid, libdnf::Logger::Level::NOTICE, "Notice message");
        log_router->write(msg_time++, pid, libdnf::Logger::Level::INFO, "Info message");
        log_router->write(msg_time++, pid, libdnf::Logger::Level::DEBUG, "Debug message");
        log_router->write(msg_time++, pid, libdnf::Logger::Level::TRACE, "Trace message");
    }

    // ====================
    // 3. Create two StreamLogger instances which replace (swap) MemoryBufferLogger instance in log_router.
    // ====================
    // Create two streams for logging. Save pointers to them. Pointers will be needed later in test of streams content.
    std::unique_ptr<std::ostringstream> log1_stream = std::make_unique<std::ostringstream>();
    std::unique_ptr<std::ostringstream> log2_stream = std::make_unique<std::ostringstream>();
    auto * log1_stream_ptr = log1_stream.get();
    auto * log2_stream_ptr = log2_stream.get();

    // Create StreamLogger instance and swap it with MemoryBufferLogger instance which was added into LogRouter before.
    std::unique_ptr<libdnf::Logger> tmp_logger = std::make_unique<libdnf::StreamLogger>(std::move(log1_stream));
    // In the log_router is registered only instance of MemoryBufferLogger just now. The index of the first logger is "0".
    log_router->swap_logger(tmp_logger, 0);

    // Create secondary StreamLogger instance and add it to LogRouter as another logger.
    log_router->add_logger(std::make_unique<libdnf::StreamLogger>(std::move(log2_stream)));

    // Test the number of registered loggers.
    CPPUNIT_ASSERT_EQUAL(log_router->get_loggers_count(), static_cast<size_t>(2));

    // ====================
    // 4. Write messages from original (replaced) MemoryBufferLoger instance into LogRouter instance. They will be routed
    //    to both attached stream loggers.
    // ====================
    dynamic_cast<libdnf::MemoryBufferLogger &>(*tmp_logger).write_to_logger(log_router.get());

    // Messages from memory logger was written to log_router. Memory logger is not needed anymore.
    tmp_logger.reset();

    // ====================
    // 5. Write aditional message into LogRouter instance.
    // ====================
    log_router->write(msg_time++, pid, libdnf::Logger::Level::INFO, "Info additional message");

    // ====================
    // 6. Check content of streams of both StreamLogger instances.
    // ====================
    CPPUNIT_ASSERT_EQUAL(log1_stream_ptr->str(), expected_loggers_content);
    CPPUNIT_ASSERT_EQUAL(log2_stream_ptr->str(), expected_loggers_content);
}
