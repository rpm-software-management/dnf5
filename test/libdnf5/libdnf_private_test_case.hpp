// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


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
