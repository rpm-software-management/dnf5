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


#ifndef TEST_LIBDNF5_LIBDNF_PRIVATE_TEST_CASE_HPP
#define TEST_LIBDNF5_LIBDNF_PRIVATE_TEST_CASE_HPP

#include "../shared/base_test_case.hpp"

#include <libdnf5/rpm/package.hpp>
#include <libdnf5/transaction/transaction_item_reason.hpp>

#include <string>

class LibdnfPrivateTestCase : public BaseTestCase {
public:
    libdnf5::rpm::Package add_system_pkg(
        const std::string & relative_path, libdnf5::transaction::TransactionItemReason reason);
};

#endif  // TEST_LIBDNF5_LIBDNF_PRIVATE_TEST_CASE_HPP
