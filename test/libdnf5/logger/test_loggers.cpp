// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "test_loggers.hpp"

#include <libdnf5/logger/log_router.hpp>
#include <libdnf5/logger/memory_buffer_logger.hpp>
#include <libdnf5/logger/stream_logger.hpp>

#include <memory>
#include <sstream>


using namespace std::chrono_literals;


CPPUNIT_TEST_SUITE_REGISTRATION(LoggersTest);


void LoggersTest::setUp() {}


void LoggersTest::tearDown() {}


// Test of logger infrastructure:
// 1. Create a LogRouter instance with one MemoryBufferLogger instances attached.
// 2. Write messages into a LogRouter instance.
// 3. Create two StreamLogger instances which replace (swap) MemoryBufferLogger instance in log_router.
// 4. Write messages from original (replaced) MemoryBufferLoger instance into LogRouter instance.
// 5. Write additional message into LogRouter instance.
// 6. Check content of streams of both StreamLogger instances.
void LoggersTest::test_loggers() {
    const char * tz = "TZ=UTC";
    putenv(const_cast<char *>(tz));
    tzset();

    auto msg_time = std::chrono::system_clock::from_time_t(1582604701);  // "2020-02-25T04:25:01Z"
    const pid_t pid = 25;

    // Text that is expected in the logs.
    const std::string expected_loggers_content =
        "2020-02-25T04:25:06+0000 [25] INFO Info message\n"
        "2020-02-25T04:25:07+0000 [25] DEBUG Debug message\n"
        "2020-02-25T04:25:08+0000 [25] TRACE Trace message\n"
        "2020-02-25T04:25:09+0000 [25] CRITICAL Critical message\n"
        "2020-02-25T04:25:10+0000 [25] ERROR Error message\n"
        "2020-02-25T04:25:11+0000 [25] WARNING Warning message\n"
        "2020-02-25T04:25:12+0000 [25] NOTICE Notice message\n"
        "2020-02-25T04:25:13+0000 [25] INFO Info message\n"
        "2020-02-25T04:25:14+0000 [25] DEBUG Debug message\n"
        "2020-02-25T04:25:15+0000 [25] TRACE Trace message\n"
        "2020-02-25T04:25:16+0000 [25] INFO Info additional message\n";

    // ====================
    // 1. Create a LogRouter instance with one MemoryBufferLogger instances attached.
    // ====================
    // Create log router.
    std::unique_ptr<libdnf5::LogRouter> log_router = std::make_unique<libdnf5::LogRouter>();
    // Create circular memory buffer logger with capacity 10 messages (4 pre-allocated from start).
    const std::size_t max_items_to_keep = 10;
    const std::size_t reserve = 4;
    log_router->add_logger(std::make_unique<libdnf5::MemoryBufferLogger>(max_items_to_keep, reserve));

    // Test the number of registered loggers.
    CPPUNIT_ASSERT_EQUAL(log_router->get_loggers_count(), static_cast<size_t>(1));

    // ====================
    // 2. Write messages into log_router. They will be routed into memory_buffer_logger.
    // ====================
    for (int i = 0; i < 2; ++i) {
        log_router->write(msg_time += 1s, pid, libdnf5::Logger::Level::CRITICAL, "Critical message");
        log_router->write(msg_time += 1s, pid, libdnf5::Logger::Level::ERROR, "Error message");
        log_router->write(msg_time += 1s, pid, libdnf5::Logger::Level::WARNING, "Warning message");
        log_router->write(msg_time += 1s, pid, libdnf5::Logger::Level::NOTICE, "Notice message");
        log_router->write(msg_time += 1s, pid, libdnf5::Logger::Level::INFO, "Info message");
        log_router->write(msg_time += 1s, pid, libdnf5::Logger::Level::DEBUG, "Debug message");
        log_router->write(msg_time += 1s, pid, libdnf5::Logger::Level::TRACE, "Trace message");
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
    std::unique_ptr<libdnf5::Logger> tmp_logger = std::make_unique<libdnf5::StreamLogger>(std::move(log1_stream));
    // In the log_router is registered only instance of MemoryBufferLogger just now. The index of the first logger is "0".
    log_router->swap_logger(tmp_logger, 0);

    // Create secondary StreamLogger instance and add it to LogRouter as another logger.
    log_router->add_logger(std::make_unique<libdnf5::StreamLogger>(std::move(log2_stream)));

    // Test the number of registered loggers.
    CPPUNIT_ASSERT_EQUAL(log_router->get_loggers_count(), static_cast<size_t>(2));

    // ====================
    // 4. Write messages from original (replaced) MemoryBufferLoger instance into LogRouter instance. They will be routed
    //    to both attached stream loggers.
    // ====================
    dynamic_cast<libdnf5::MemoryBufferLogger &>(*tmp_logger).write_to_logger(*log_router);

    // Messages from memory logger was written to log_router. Memory logger is not needed anymore.
    tmp_logger.reset();

    // ====================
    // 5. Write additional message into LogRouter instance.
    // ====================
    log_router->write(msg_time += 1s, pid, libdnf5::Logger::Level::INFO, "Info additional message");

    // ====================
    // 6. Check content of streams of both StreamLogger instances.
    // ====================
    CPPUNIT_ASSERT_EQUAL(log1_stream_ptr->str(), expected_loggers_content);
    CPPUNIT_ASSERT_EQUAL(log2_stream_ptr->str(), expected_loggers_content);
}
