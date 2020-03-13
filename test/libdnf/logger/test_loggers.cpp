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
// 1. Write messages into a circular MemoryBufferLogger instance.
// 2. Create a LogRouter instance with two StreamLogger instances attached.
// 3. Write messages from MemoryBufferLoger instance into LogRouter instance.
// 4. Write aditional message into LogRouter instance.
// 5. Check content of streams of both StreamLogger instances.
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
    // 1. Write messages into a circular MemoryBufferLogger instance.
    // ====================
    // Create circular memory buffer logger with capacity 10 messages (4 pre-allocated from start).
    const std::size_t max_items_to_keep = 10;
    const std::size_t reserve = 4;
    std::unique_ptr<libdnf::MemoryBufferLogger> memory_buffer_logger =
        std::make_unique<libdnf::MemoryBufferLogger>(max_items_to_keep, reserve);

    // Write messages into memory buffer loger.
    for (int i = 0; i < 2; ++i) {
        memory_buffer_logger->write(msg_time++, pid, libdnf::Logger::Level::CRITICAL, "Critical message");
        memory_buffer_logger->write(msg_time++, pid, libdnf::Logger::Level::ERROR, "Error message");
        memory_buffer_logger->write(msg_time++, pid, libdnf::Logger::Level::WARNING, "Warning message");
        memory_buffer_logger->write(msg_time++, pid, libdnf::Logger::Level::NOTICE, "Notice message");
        memory_buffer_logger->write(msg_time++, pid, libdnf::Logger::Level::INFO, "Info message");
        memory_buffer_logger->write(msg_time++, pid, libdnf::Logger::Level::DEBUG, "Debug message");
        memory_buffer_logger->write(msg_time++, pid, libdnf::Logger::Level::TRACE, "Trace message");
    }

    // ====================
    // 2. Create a LogRouter instance with two StreamLogger instances attached.
    // ====================
    // Create log router.
    std::unique_ptr<libdnf::LogRouter> log_router = std::make_unique<libdnf::LogRouter>();

    // Create two streams for logging. Save pointers to them. Pointers will be needed later in test of streams content.
    std::unique_ptr<std::ostringstream> log1_stream = std::make_unique<std::ostringstream>();
    std::unique_ptr<std::ostringstream> log2_stream = std::make_unique<std::ostringstream>();
    auto * log1_stream_ptr = log1_stream.get();
    auto * log2_stream_ptr = log2_stream.get();

    // Create StreamLogger instances and attach them to LogRouter instance.
    log_router->add_logger(std::make_unique<libdnf::StreamLogger>(std::move(log1_stream)));
    log_router->add_logger(std::make_unique<libdnf::StreamLogger>(std::move(log2_stream)));

    // ====================
    // 3. Write messages from MemoryBufferLoger instance into LogRouter instance.
    // ====================
    memory_buffer_logger->write_to_logger(log_router.get());

    // Messages from memory logger was written to log_router. Memory logger is not needed anymore.
    memory_buffer_logger.reset();

    // ====================
    // 4. Write aditional message into LogRouter instance.
    // ====================
    log_router->write(msg_time++, pid, libdnf::Logger::Level::INFO, "Info additional message");

    // ====================
    // 5. Check content of streams of both StreamLogger instances.
    // ====================
    CPPUNIT_ASSERT_EQUAL(log1_stream_ptr->str(), expected_loggers_content);
    CPPUNIT_ASSERT_EQUAL(log2_stream_ptr->str(), expected_loggers_content);
}
