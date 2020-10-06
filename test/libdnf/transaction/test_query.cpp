/*
Copyright (C) 2017-2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "test_query.hpp"

#include "libdnf/common/sack/query_cmp.hpp"
#include "libdnf/transaction/transaction.hpp"

#include <string>


using namespace libdnf::transaction;


CPPUNIT_TEST_SUITE_REGISTRATION(TransactionQueryTest);


void TransactionQueryTest::test_ifilter_id_eq() {
    auto base = new_base();

    // create a new empty transaction
    auto trans = base->get_transaction_sack().new_transaction();

    // save the transaction
    trans->begin();
    trans->finish(TransactionState::DONE);

    // create a new Base to force reading the transaction from disk
    auto base2 = new_base();

    // get the written transaction
    auto q2 = base2->get_transaction_sack().new_query();
    q2.ifilter_id(libdnf::sack::QueryCmp::EXACT, trans->get_id());
    auto trans2 = q2.get();
}


void TransactionQueryTest::test_ifilter_id_eq_parallel_queries() {
    auto base = new_base();

    // create a new empty transaction
    auto trans = base->get_transaction_sack().new_transaction();

    // save the transaction
    trans->begin();
    trans->finish(TransactionState::DONE);

    // create a new Base to force reading the transaction from disk
    auto base2 = new_base();

    // the sack is empty
    CPPUNIT_ASSERT_EQUAL(0LU, base2->get_transaction_sack().get_data().size());

    // create 2 queries that are empty, none of them loaded any transaction yet
    auto q2 = base2->get_transaction_sack().new_query();
    auto q3 = base2->get_transaction_sack().new_query();

    q2.ifilter_id(libdnf::sack::QueryCmp::EXACT, trans->get_id());
    auto trans2 = q2.get();

    // one item loaded into the sack
    CPPUNIT_ASSERT_EQUAL(1LU, base2->get_transaction_sack().get_data().size());

    q3.ifilter_id(libdnf::sack::QueryCmp::EXACT, trans->get_id());
    auto trans3 = q3.get();

    // query reused the existing item in the sack
    CPPUNIT_ASSERT_EQUAL(1LU, base2->get_transaction_sack().get_data().size());

    // finally compare the transactions
    CPPUNIT_ASSERT(trans2 == trans3);
}
