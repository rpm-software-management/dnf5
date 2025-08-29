// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_TEST_LOGGER_HPP
#define TEST_LIBDNF5_TEST_LOGGER_HPP

#include <libdnf5/logger/memory_buffer_logger.hpp>

// Global logger used in many tests.
// Logging from libdnf5::Base is routed to it using LoggerRedirector.
extern libdnf5::MemoryBufferLogger test_logger;

#endif  // TEST_LIBDNF5_TEST_LOGGER_HPP
