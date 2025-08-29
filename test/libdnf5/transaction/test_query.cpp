// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#include "test_query.hpp"

#include "../shared/private_accessor.hpp"

#include <libdnf5/transaction/transaction.hpp>

#include <string>


using namespace libdnf5::transaction;


CPPUNIT_TEST_SUITE_REGISTRATION(TransactionQueryTest);

namespace {

// Allows accessing private methods
create_private_getter_template;
create_getter(start, &libdnf5::transaction::Transaction::start);
create_getter(finish, &libdnf5::transaction::Transaction::finish);
create_getter(new_transaction, &libdnf5::transaction::TransactionHistory::new_transaction);

}  //namespace

void TransactionQueryTest::test_filter_id_eq() {
    auto base = new_base();

    // create a new empty transaction
    libdnf5::transaction::TransactionHistory history(base->get_weak_ptr());
    auto trans = (history.*get(new_transaction{}))();

    // save the transaction
    (trans.*get(start{}))();
    (trans.*get(finish{}))(TransactionState::OK);

    // create a new Base to force reading the transaction from disk
    auto base2 = new_base();

    // get the written transaction
    libdnf5::transaction::TransactionHistory history2(base2->get_weak_ptr());
    auto ts_list = history2.list_transactions({trans.get_id()});
    CPPUNIT_ASSERT_EQUAL((size_t)1, ts_list.size());
}
