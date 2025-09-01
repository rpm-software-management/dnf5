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


#include "test_transaction.hpp"

#include "../shared/private_accessor.hpp"

#include <libdnf5/transaction/transaction.hpp>

#include <string>


using namespace libdnf5::transaction;


CPPUNIT_TEST_SUITE_REGISTRATION(TransactionTest);

namespace {

// Allows accessing private methods
create_private_getter_template;
create_getter(set_dt_start, &libdnf5::transaction::Transaction::set_dt_start);
create_getter(set_dt_end, &libdnf5::transaction::Transaction::set_dt_end);
create_getter(set_rpmdb_version_begin, &libdnf5::transaction::Transaction::set_rpmdb_version_begin);
create_getter(set_rpmdb_version_end, &libdnf5::transaction::Transaction::set_rpmdb_version_end);
create_getter(set_releasever, &libdnf5::transaction::Transaction::set_releasever);
create_getter(set_user_id, &libdnf5::transaction::Transaction::set_user_id);
create_getter(set_description, &libdnf5::transaction::Transaction::set_description);
create_getter(set_state, &libdnf5::transaction::Transaction::set_state);
create_getter(set_id, &libdnf5::transaction::Transaction::set_id);
create_getter(start, &libdnf5::transaction::Transaction::start);
create_getter(finish, &libdnf5::transaction::Transaction::finish);
create_getter(new_transaction, &libdnf5::transaction::TransactionHistory::new_transaction);

}  //namespace

static Transaction create_transaction(libdnf5::Base & base, int nr) {
    libdnf5::transaction::TransactionHistory history(base);
    auto trans = (history.*get(new_transaction{}))();
    (trans.*get(set_dt_start{}))(nr * 10 + 1);
    (trans.*get(set_dt_end{}))(nr * 10 + 2);
    (trans.*get(set_rpmdb_version_begin{}))(fmt::format("ts {} begin", nr));
    (trans.*get(set_rpmdb_version_end{}))(fmt::format("ts {} end", nr));
    (trans.*get(set_releasever{}))("26");
    (trans.*get(set_user_id{}))(1000);
    (trans.*get(set_description{}))("dnf install foo");

    return trans;
}


void TransactionTest::test_save_load() {
    auto base = new_base();

    auto trans = create_transaction(*base, 1);
    (trans.*get(start{}))();
    (trans.*get(finish{}))(TransactionState::OK);

    // load the saved transaction from database and compare values
    auto base2 = new_base();

    libdnf5::transaction::TransactionHistory history2(base2->get_weak_ptr());
    auto ts_list = history2.list_transactions({trans.get_id()});
    CPPUNIT_ASSERT_EQUAL((size_t)1, ts_list.size());

    auto trans2 = ts_list[0];

    CPPUNIT_ASSERT_EQUAL(trans.get_id(), trans2.get_id());
    CPPUNIT_ASSERT_EQUAL(trans.get_dt_start(), trans2.get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans.get_dt_end(), trans2.get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans.get_rpmdb_version_begin(), trans2.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans.get_rpmdb_version_end(), trans2.get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans.get_releasever(), trans2.get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans.get_user_id(), trans2.get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans.get_description(), trans2.get_description());
    CPPUNIT_ASSERT_EQUAL(trans.get_state(), trans2.get_state());
}


void TransactionTest::test_second_start_raises() {
    auto base = new_base();
    libdnf5::transaction::TransactionHistory history(base->get_weak_ptr());
    auto trans = (history.*get(new_transaction{}))();
    (trans.*get(start{}))();
    // 2nd begin must throw an exception
    CPPUNIT_ASSERT_THROW((trans.*get(start{}))(), libdnf5::RuntimeError);
}


void TransactionTest::test_save_with_specified_id_raises() {
    auto base = new_base();
    libdnf5::transaction::TransactionHistory history(base->get_weak_ptr());
    auto trans = (history.*get(new_transaction{}))();
    (trans.*get(set_id{}))(1);
    // it is not allowed to save a transaction with arbitrary ID
    CPPUNIT_ASSERT_THROW((trans.*get(start{}))(), libdnf5::RuntimeError);
}


void TransactionTest::test_update() {
    auto base = new_base();

    auto trans = create_transaction(*base, 1);
    (trans.*get(start{}))();

    // modify the values after the initial save
    (trans.*get(set_dt_start{}))(12);
    (trans.*get(set_dt_end{}))(22);
    (trans.*get(set_rpmdb_version_begin{}))("begin_2");
    (trans.*get(set_rpmdb_version_end{}))("end_2");
    (trans.*get(set_releasever{}))("26_2");
    (trans.*get(set_user_id{}))(10002);
    (trans.*get(set_description{}))("dnf install foo_2");
    (trans.*get(set_state{}))(TransactionState::ERROR);
    (trans.*get(finish{}))(TransactionState::OK);

    // load the transaction from the database
    auto base2 = new_base();
    libdnf5::transaction::TransactionHistory history2(base2->get_weak_ptr());
    auto ts_list = history2.list_transactions({trans.get_id()});
    CPPUNIT_ASSERT_EQUAL((size_t)1, ts_list.size());

    auto trans2 = ts_list[0];

    // check if the values saved during trans.finish() match
    CPPUNIT_ASSERT_EQUAL(trans.get_id(), trans2.get_id());
    CPPUNIT_ASSERT_EQUAL(trans.get_dt_start(), trans2.get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans.get_dt_end(), trans2.get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans.get_rpmdb_version_begin(), trans2.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans.get_rpmdb_version_end(), trans2.get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans.get_releasever(), trans2.get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans.get_user_id(), trans2.get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans.get_description(), trans2.get_description());
    CPPUNIT_ASSERT_EQUAL(trans.get_state(), trans2.get_state());
}


void TransactionTest::test_compare() {
    auto base = new_base();

    // test operator ==, > and <
    libdnf5::transaction::TransactionHistory history(base->get_weak_ptr());
    auto first = (history.*get(new_transaction{}))();
    auto second = (history.*get(new_transaction{}))();

    // equal id
    (first.*get(set_id{}))(1);
    (second.*get(set_id{}))(1);
    CPPUNIT_ASSERT(first == second);

    // compare by id
    (second.*get(set_id{}))(2);
    CPPUNIT_ASSERT(first > second);
    CPPUNIT_ASSERT(second < first);

    // equal id and dt_begin
    (first.*get(set_id{}))(1);
    (second.*get(set_id{}))(1);
    (first.*get(set_dt_start{}))(1);
    (second.*get(set_dt_start{}))(1);
    CPPUNIT_ASSERT(first == second);
}


void TransactionTest::test_select_all() {
    auto base = new_base();

    auto trans1 = create_transaction(*base, 1);
    (trans1.*get(start{}))();
    (trans1.*get(finish{}))(TransactionState::OK);

    auto trans2 = create_transaction(*base, 2);
    (trans2.*get(start{}))();
    (trans2.*get(finish{}))(TransactionState::OK);

    auto trans3 = create_transaction(*base, 3);
    (trans3.*get(start{}))();
    (trans3.*get(finish{}))(TransactionState::OK);

    // load the saved transaction from database and compare values
    libdnf5::transaction::TransactionHistory history(base->get_weak_ptr());
    auto ts_list = history.list_all_transactions();
    CPPUNIT_ASSERT_EQUAL((size_t)3, ts_list.size());

    auto trans1_loaded = ts_list[0];

    CPPUNIT_ASSERT_EQUAL(trans1.get_id(), trans1_loaded.get_id());
    CPPUNIT_ASSERT_EQUAL(trans1.get_dt_start(), trans1_loaded.get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans1.get_dt_end(), trans1_loaded.get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans1.get_rpmdb_version_begin(), trans1_loaded.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans1.get_rpmdb_version_end(), trans1_loaded.get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans1.get_releasever(), trans1_loaded.get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans1.get_user_id(), trans1_loaded.get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans1.get_description(), trans1_loaded.get_description());
    CPPUNIT_ASSERT_EQUAL(trans1.get_state(), trans1_loaded.get_state());

    auto trans2_loaded = ts_list[1];

    CPPUNIT_ASSERT_EQUAL(trans2.get_id(), trans2_loaded.get_id());
    CPPUNIT_ASSERT_EQUAL(trans2.get_dt_start(), trans2_loaded.get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans2.get_dt_end(), trans2_loaded.get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans2.get_rpmdb_version_begin(), trans2_loaded.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans2.get_rpmdb_version_end(), trans2_loaded.get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans2.get_releasever(), trans2_loaded.get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans2.get_user_id(), trans2_loaded.get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans2.get_description(), trans2_loaded.get_description());
    CPPUNIT_ASSERT_EQUAL(trans2.get_state(), trans2_loaded.get_state());

    auto trans3_loaded = ts_list[2];

    CPPUNIT_ASSERT_EQUAL(trans3.get_id(), trans3_loaded.get_id());
    CPPUNIT_ASSERT_EQUAL(trans3.get_dt_start(), trans3_loaded.get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans3.get_dt_end(), trans3_loaded.get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans3.get_rpmdb_version_begin(), trans3_loaded.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans3.get_rpmdb_version_end(), trans3_loaded.get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans3.get_releasever(), trans3_loaded.get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans3.get_user_id(), trans3_loaded.get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans3.get_description(), trans3_loaded.get_description());
    CPPUNIT_ASSERT_EQUAL(trans3.get_state(), trans3_loaded.get_state());
}


void TransactionTest::test_select_multiple() {
    auto base = new_base();

    auto trans1 = create_transaction(*base, 1);
    (trans1.*get(start{}))();
    (trans1.*get(finish{}))(TransactionState::OK);

    auto trans2 = create_transaction(*base, 2);
    (trans2.*get(start{}))();
    (trans2.*get(finish{}))(TransactionState::OK);

    auto trans3 = create_transaction(*base, 3);
    (trans3.*get(start{}))();
    (trans3.*get(finish{}))(TransactionState::OK);

    // load the saved transaction from database and compare values
    libdnf5::transaction::TransactionHistory history(base->get_weak_ptr());
    auto ts_list = history.list_transactions({1, 3});
    CPPUNIT_ASSERT_EQUAL((size_t)2, ts_list.size());

    auto trans1_loaded = ts_list[0];

    CPPUNIT_ASSERT_EQUAL(trans1.get_id(), trans1_loaded.get_id());
    CPPUNIT_ASSERT_EQUAL(trans1.get_dt_start(), trans1_loaded.get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans1.get_dt_end(), trans1_loaded.get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans1.get_rpmdb_version_begin(), trans1_loaded.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans1.get_rpmdb_version_end(), trans1_loaded.get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans1.get_releasever(), trans1_loaded.get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans1.get_user_id(), trans1_loaded.get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans1.get_description(), trans1_loaded.get_description());
    CPPUNIT_ASSERT_EQUAL(trans1.get_state(), trans1_loaded.get_state());

    auto trans3_loaded = ts_list[1];

    CPPUNIT_ASSERT_EQUAL(trans3.get_id(), trans3_loaded.get_id());
    CPPUNIT_ASSERT_EQUAL(trans3.get_dt_start(), trans3_loaded.get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans3.get_dt_end(), trans3_loaded.get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans3.get_rpmdb_version_begin(), trans3_loaded.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans3.get_rpmdb_version_end(), trans3_loaded.get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans3.get_releasever(), trans3_loaded.get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans3.get_user_id(), trans3_loaded.get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans3.get_description(), trans3_loaded.get_description());
    CPPUNIT_ASSERT_EQUAL(trans3.get_state(), trans3_loaded.get_state());
}


void TransactionTest::test_select_range() {
    auto base = new_base();

    auto trans1 = create_transaction(*base, 1);
    (trans1.*get(start{}))();
    (trans1.*get(finish{}))(TransactionState::OK);

    auto trans2 = create_transaction(*base, 2);
    (trans2.*get(start{}))();
    (trans2.*get(finish{}))(TransactionState::OK);

    auto trans3 = create_transaction(*base, 3);
    (trans3.*get(start{}))();
    (trans3.*get(finish{}))(TransactionState::OK);

    // load the saved transaction from database and compare values
    libdnf5::transaction::TransactionHistory history(base->get_weak_ptr());
    auto ts_list = history.list_transactions(1, 2);
    CPPUNIT_ASSERT_EQUAL((size_t)2, ts_list.size());

    auto trans1_loaded = ts_list[0];

    CPPUNIT_ASSERT_EQUAL(trans1.get_id(), trans1_loaded.get_id());
    CPPUNIT_ASSERT_EQUAL(trans1.get_dt_start(), trans1_loaded.get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans1.get_dt_end(), trans1_loaded.get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans1.get_rpmdb_version_begin(), trans1_loaded.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans1.get_rpmdb_version_end(), trans1_loaded.get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans1.get_releasever(), trans1_loaded.get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans1.get_user_id(), trans1_loaded.get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans1.get_description(), trans1_loaded.get_description());
    CPPUNIT_ASSERT_EQUAL(trans1.get_state(), trans1_loaded.get_state());

    auto trans2_loaded = ts_list[1];

    CPPUNIT_ASSERT_EQUAL(trans2.get_id(), trans2_loaded.get_id());
    CPPUNIT_ASSERT_EQUAL(trans2.get_dt_start(), trans2_loaded.get_dt_start());
    CPPUNIT_ASSERT_EQUAL(trans2.get_dt_end(), trans2_loaded.get_dt_end());
    CPPUNIT_ASSERT_EQUAL(trans2.get_rpmdb_version_begin(), trans2_loaded.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(trans2.get_rpmdb_version_end(), trans2_loaded.get_rpmdb_version_end());
    CPPUNIT_ASSERT_EQUAL(trans2.get_releasever(), trans2_loaded.get_releasever());
    CPPUNIT_ASSERT_EQUAL(trans2.get_user_id(), trans2_loaded.get_user_id());
    CPPUNIT_ASSERT_EQUAL(trans2.get_description(), trans2_loaded.get_description());
    CPPUNIT_ASSERT_EQUAL(trans2.get_state(), trans2_loaded.get_state());
}
