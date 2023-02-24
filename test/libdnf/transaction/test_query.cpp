/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "test_query.hpp"

#include "../shared/private_accessor.hpp"

#include "libdnf/transaction/transaction.hpp"

#include <string>


using namespace libdnf::transaction;


CPPUNIT_TEST_SUITE_REGISTRATION(TransactionQueryTest);

namespace {

// Allows accessing private methods
create_private_getter_template;
create_getter(start, &libdnf::transaction::Transaction::start);
create_getter(finish, &libdnf::transaction::Transaction::finish);
create_getter(new_transaction, &libdnf::transaction::TransactionHistory::new_transaction);

}  //namespace

void TransactionQueryTest::test_filter_id_eq() {
    auto base = new_base();

    // create a new empty transaction
    auto trans = (*(base->get_transaction_history()).*get(new_transaction{}))();

    // save the transaction
    (trans.*get(start{}))();
    (trans.*get(finish{}))(TransactionState::OK);

    // create a new Base to force reading the transaction from disk
    auto base2 = new_base();

    // get the written transaction
    auto ts_list = base2->get_transaction_history()->list_transactions({trans.get_id()});
    CPPUNIT_ASSERT_EQUAL((size_t)1, ts_list.size());
}
