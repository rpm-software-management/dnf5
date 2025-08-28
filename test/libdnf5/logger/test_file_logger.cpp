// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "test_file_logger.hpp"

#include <libdnf5/logger/factory.hpp>


using namespace std::filesystem;


CPPUNIT_TEST_SUITE_REGISTRATION(FileLoggerTest);

constexpr const char * FILE_LOGGER_FILENAME = "dnf5.log";

void FileLoggerTest::setUp() {
    BaseTestCase::setUp();

    auto & config = base.get_config();
    auto installroot = path(config.get_installroot_option().get_value());
    auto temp_logdir = path("/var/log/FileLoggerTestLogDir");
    config.get_logdir_option().set(installroot / temp_logdir.relative_path());

    full_log_path = installroot / temp_logdir.relative_path() / FILE_LOGGER_FILENAME;
}


void FileLoggerTest::tearDown() {
    BaseTestCase::tearDown();
    remove_all(full_log_path.parent_path());
}


void FileLoggerTest::test_file_logger_create_name() {
    CPPUNIT_ASSERT(!exists(full_log_path));
    auto file_logger = libdnf5::create_file_logger(base, FILE_LOGGER_FILENAME);
    CPPUNIT_ASSERT(exists(full_log_path));
}


void FileLoggerTest::test_file_logger_add() {
    auto log_router = base.get_logger();
    auto loggers_count_before = log_router->get_loggers_count();
    auto file_logger = libdnf5::create_file_logger(base, FILE_LOGGER_FILENAME);
    log_router->add_logger(std::move(file_logger));
    CPPUNIT_ASSERT_EQUAL(loggers_count_before + 1, log_router->get_loggers_count());
}
