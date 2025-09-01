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
