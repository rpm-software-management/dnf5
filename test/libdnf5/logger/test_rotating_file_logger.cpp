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

#include "test_rotating_file_logger.hpp"

#include "libdnf5/utils/fs/file.hpp"
#include "libdnf5/utils/fs/temp.hpp"

#include <libdnf5/logger/rotating_file_logger.hpp>

using namespace std::chrono_literals;

using LogLevel = libdnf5::Logger::Level;

CPPUNIT_TEST_SUITE_REGISTRATION(RotatingFileLoggerTest);


void RotatingFileLoggerTest::setUp() {}


void RotatingFileLoggerTest::tearDown() {}


void RotatingFileLoggerTest::test() {
    const char * const tz = "TZ=UTC";
    putenv(const_cast<char *>(tz));
    tzset();

    auto msg_time = std::chrono::system_clock::from_time_t(1582604701);  // "2020-02-25T04:25:01Z"
    const pid_t pid = 25;

    // ====================
    // Expected contents of log files.
    // ====================
    const std::string expected_base_file_content = "2020-02-25T04:25:10+0000 [25] WARNING 9: Last message\n";

    const std::string expected_rotated_file_1_content =
        "2020-02-25T04:25:08+0000 [25] CRITICAL 7: Next message\n"
        "2020-02-25T04:25:09+0000 [25] ERROR 8: And another message is here\n";

    const std::string expected_rotated_file_2_content =
        "2020-02-25T04:25:07+0000 [25] TRACE 6: Message longer than \"max\" logger file size. The logger file will "
        "only contain this entire message. The message will not be truncated\n";

    const std::string expected_rotated_file_3_content =
        "2020-02-25T04:25:04+0000 [25] WARNING 3: Message\n"
        "2020-02-25T04:25:05+0000 [25] NOTICE 4: Message\n"
        "2020-02-25T04:25:06+0000 [25] INFO 5: Message\n";

    // Create a directory for the log files.
    libdnf5::utils::fs::TempDir temp_logdir("libdnf_unittest_rotating_logger");
    // Create a path to the base log file.
    const auto base_log_file_path = temp_logdir.get_path() / "rotated.log";

    // ====================
    // Create a RotatingFileLogger instance and write messages.
    // ====================
    {
        constexpr std::size_t MAX_BYTES = 150;
        constexpr std::size_t BACKUP_COUNT = 3;
        libdnf5::RotatingFileLogger rotating_file_logger(base_log_file_path, MAX_BYTES, BACKUP_COUNT);

        rotating_file_logger.write(msg_time += 1s, pid, LogLevel::CRITICAL, "1: First message rotated out");
        rotating_file_logger.write(msg_time += 1s, pid, LogLevel::ERROR, "2: Secondth message rotated out");
        rotating_file_logger.write(msg_time += 1s, pid, LogLevel::WARNING, "3: Message");
        rotating_file_logger.write(msg_time += 1s, pid, LogLevel::NOTICE, "4: Message");
        rotating_file_logger.write(msg_time += 1s, pid, LogLevel::INFO, "5: Message");
        rotating_file_logger.write(
            msg_time += 1s,
            pid,
            LogLevel::TRACE,
            "6: Message longer than \"max\" logger file size. The logger file will only contain this entire message. "
            "The message will not be truncated");
        rotating_file_logger.write(msg_time += 1s, pid, LogLevel::CRITICAL, "7: Next message");
        rotating_file_logger.write(msg_time += 1s, pid, LogLevel::ERROR, "8: And another message is here");
        rotating_file_logger.write(msg_time += 1s, pid, LogLevel::WARNING, "9: Last message");
    }

    // ====================
    // Check content of logger files.
    // ====================
    auto read_content = libdnf5::utils::fs::File(base_log_file_path, "r").read();
    CPPUNIT_ASSERT_EQUAL(expected_base_file_content, read_content);

    read_content = libdnf5::utils::fs::File(base_log_file_path.string() + ".1", "r").read();
    CPPUNIT_ASSERT_EQUAL(expected_rotated_file_1_content, read_content);

    read_content = libdnf5::utils::fs::File(base_log_file_path.string() + ".2", "r").read();
    CPPUNIT_ASSERT_EQUAL(expected_rotated_file_2_content, read_content);

    read_content = libdnf5::utils::fs::File(base_log_file_path.string() + ".3", "r").read();
    CPPUNIT_ASSERT_EQUAL(expected_rotated_file_3_content, read_content);
}
