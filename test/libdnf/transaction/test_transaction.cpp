/*
Copyright (C) 2017-2021 Red Hat, Inc.

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


#include "test_transaction.hpp"

#include "libdnf/common/sack/query_cmp.hpp"
#include "libdnf/transaction/transaction.hpp"

#include <string>


using namespace libdnf::transaction;


CPPUNIT_TEST_SUITE_REGISTRATION(TransactionTest);


static TransactionWeakPtr create_transaction(libdnf::Base & base) {
    auto trans = base.get_transaction_sack()->new_transaction();
    trans->set_dt_start(1);
    trans->set_dt_end(2);
    trans->set_rpmdb_version_begin("begin");
    trans->set_rpmdb_version_end("end");
    trans->set_releasever("26");
    trans->set_user_id(1000);
    trans->set_cmdline("dnf install foo");

    trans->add_runtime_package("rpm-4.14.2-1.fc29.x86_64");
    trans->add_runtime_package("dnf-3.5.1-1.fc29.noarch");
    // test adding a duplicate; only a single occurrence of the rpm is expected
    trans->add_runtime_package("rpm-4.14.2-1.fc29.x86_64");
    CPPUNIT_ASSERT_EQUAL(2UL, trans->get_runtime_packages().size());

    return trans;
}


void TransactionTest::test_save_load() {
    auto base = new_base();

    auto trans = create_transaction(*base);
    trans->start();
    trans->finish(TransactionState::DONE);

    // load the saved transaction from database and compare values
    auto base2 = new_base();
    libdnf::transaction::TransactionQuery q2(*base2);
    q2.filter_id(trans->get_id());
    auto trans2 = q2.get();

    CPPUNIT_ASSERT_EQUAL(trans->get_id(), trans2->get_id());
    CPPUNIT_ASSERT_EQUAL(trans->get_dt_start(), trans2->get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans->get_dt_end(), trans2->get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans->get_rpmdb_version_begin(), trans2->get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans->get_rpmdb_version_end(), trans2->get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans->get_releasever(), trans2->get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans->get_user_id(), trans2->get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans->get_cmdline(), trans2->get_cmdline());
    CPPUNIT_ASSERT_EQUAL(trans->get_state(), trans2->get_state());
    CPPUNIT_ASSERT_EQUAL(2UL, trans2->get_runtime_packages().size());
}


void TransactionTest::test_second_start_raises() {
    auto base = new_base();
    auto trans = base->get_transaction_sack()->new_transaction();
    trans->start();
    // 2nd begin must throw an exception
    CPPUNIT_ASSERT_THROW(trans->start(), std::runtime_error);
}


void TransactionTest::test_save_with_specified_id_raises() {
    auto base = new_base();
    auto trans = base->get_transaction_sack()->new_transaction();
    trans->set_id(1);
    // it is not allowed to save a transaction with arbitrary ID
    CPPUNIT_ASSERT_THROW(trans->start(), std::runtime_error);
}


void TransactionTest::test_update() {
    auto base = new_base();

    auto trans = create_transaction(*base);
    trans->start();

    // modify the values after the initial save
    trans->set_dt_start(12);
    trans->set_dt_end(22);
    trans->set_rpmdb_version_begin("begin_2");
    trans->set_rpmdb_version_end("end_2");
    trans->set_releasever("26_2");
    trans->set_user_id(10002);
    trans->set_cmdline("dnf install foo_2");
    trans->set_state(TransactionState::ERROR);
    trans->finish(TransactionState::DONE);

    // load the transction from the database
    auto base2 = new_base();
    libdnf::transaction::TransactionQuery q2(*base2);
    q2.filter_id(trans->get_id());
    auto trans2 = q2.get();

    // check if the values saved during trans->finish() match
    CPPUNIT_ASSERT_EQUAL(trans->get_id(), trans2->get_id());
    CPPUNIT_ASSERT_EQUAL(trans->get_dt_start(), trans2->get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans->get_dt_end(), trans2->get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans->get_rpmdb_version_begin(), trans2->get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans->get_rpmdb_version_end(), trans2->get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans->get_releasever(), trans2->get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans->get_user_id(), trans2->get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans->get_cmdline(), trans2->get_cmdline());
    CPPUNIT_ASSERT_EQUAL(trans->get_state(), trans2->get_state());
    CPPUNIT_ASSERT_EQUAL(2UL, trans2->get_runtime_packages().size());
}


void TransactionTest::test_compare() {
    auto base = new_base();

    // test operator ==, > and <
    auto transaction_sack = base->get_transaction_sack();
    auto first = transaction_sack->new_transaction();
    auto second = transaction_sack->new_transaction();

    // equal id
    first->set_id(1);
    second->set_id(1);
    CPPUNIT_ASSERT(*first == *second);

    // compare by id
    second->set_id(2);
    CPPUNIT_ASSERT(*first > *second);
    CPPUNIT_ASSERT(*second < *first);

    // equal id and dt_begin
    first->set_id(1);
    second->set_id(1);
    first->set_dt_start(1);
    second->set_dt_start(1);
    CPPUNIT_ASSERT(*first == *second);

    // equal id, compare by dt_begin
    second->set_dt_start(2);
    CPPUNIT_ASSERT(*first > *second);
    CPPUNIT_ASSERT(*second < *first);
}
