// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_TESTCASE_FIXTURE_HPP
#define TEST_LIBDNF5_TESTCASE_FIXTURE_HPP

#include "libdnf5/utils/fs/temp.hpp"

#include <cppunit/TestCase.h>
#include <libdnf5/base/base.hpp>


class TestCaseFixture : public CppUnit::TestCase {
public:
    void setUp() override;
    void tearDown() override;

    std::unique_ptr<libdnf5::Base> get_preconfigured_base();

    // Only gets created if get_preconfigured_base() is called
    std::unique_ptr<libdnf5::utils::fs::TempDir> temp_dir;
};


#endif  // TEST_LIBDNF5_TESTCASE_FIXTURE_HPP
