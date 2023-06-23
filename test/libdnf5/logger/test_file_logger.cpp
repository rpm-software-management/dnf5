/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "test_file_logger.hpp"

#include <libdnf5/logger/factory.hpp>


using namespace std::filesystem;


CPPUNIT_TEST_SUITE_REGISTRATION(FileLoggerTest);


void FileLoggerTest::setUp() {
    BaseTestCase::setUp();

    auto & config = base.get_config();
    auto & installroot = config.get_installroot_option().get_value();
    auto & temp_logdir = "/var/log/FileLoggerTestLogDir";
    config.get_logdir_option().set(temp_logdir);

    full_log_path = path(installroot) / path(temp_logdir).relative_path() / libdnf5::FILE_LOGGER_FILENAME;
}


void FileLoggerTest::tearDown() {
    BaseTestCase::tearDown();
    remove_all(full_log_path.parent_path());
}


void FileLoggerTest::test_file_logger_create() {
    CPPUNIT_ASSERT(!exists(full_log_path));
    auto file_logger = libdnf5::create_file_logger(base);
    CPPUNIT_ASSERT(exists(full_log_path));
}


void FileLoggerTest::test_file_logger_add() {
    auto log_router = base.get_logger();
    auto loggers_count_before = log_router->get_loggers_count();
    auto file_logger = libdnf5::create_file_logger(base);
    log_router->add_logger(std::move(file_logger));
    CPPUNIT_ASSERT_EQUAL(loggers_count_before + 1, log_router->get_loggers_count());
}
