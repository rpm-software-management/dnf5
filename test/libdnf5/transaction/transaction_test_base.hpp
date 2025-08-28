// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef LIBDNF5_TEST_TRANSACTION_TRANSACTION_TEST_BASE_HPP
#define LIBDNF5_TEST_TRANSACTION_TRANSACTION_TEST_BASE_HPP


#include "libdnf5/utils/fs/temp.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/base/base.hpp>

#include <memory>


class TransactionTestBase : public CppUnit::TestCase {
public:
    void setUp() override;
    void tearDown() override;

protected:
    std::unique_ptr<libdnf5::Base> new_base();
    std::unique_ptr<libdnf5::utils::fs::TempDir> temp_dir;
};


#endif  // LIBDNF5_TEST_TRANSACTION_TRANSACTION_TEST_BASE_HPP
