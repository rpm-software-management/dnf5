// Copyright Contributors to the DNF5 project.
// Copyright (C) 2021 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later


#ifndef TEST_LIBDNF5_SYSTEM_STATE_HPP
#define TEST_LIBDNF5_SYSTEM_STATE_HPP


#include "../shared/base_test_case.hpp"
#include "system/state.hpp"

#include "libdnf5/utils/fs/temp.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/comps/group/package.hpp>

#include <memory>


class StateTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(StateTest);
    CPPUNIT_TEST(test_state_version);
    CPPUNIT_TEST(test_state_read);
    CPPUNIT_TEST(test_state_write);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_state_version();
    void test_state_read();
    void test_state_write();

    std::unique_ptr<libdnf5::utils::fs::TempDir> temp_dir;
};


#endif
