// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#include "transaction_test_base.hpp"


void TransactionTestBase::setUp() {
    temp_dir = std::make_unique<libdnf5::utils::fs::TempDir>("libdnf_unittest");
}


void TransactionTestBase::tearDown() {}


std::unique_ptr<libdnf5::Base> TransactionTestBase::new_base() {
    auto new_base = std::make_unique<libdnf5::Base>();
    new_base->get_config().get_transaction_history_dir_option().set(temp_dir->get_path());
    return new_base;
}
