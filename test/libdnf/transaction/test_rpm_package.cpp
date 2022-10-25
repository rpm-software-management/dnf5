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


#include "test_rpm_package.hpp"

#include "private_accessor.hpp"

#include "libdnf/common/sack/query_cmp.hpp"
#include "libdnf/transaction/rpm_package.hpp"
#include "libdnf/transaction/transaction.hpp"

#include <string>


using namespace libdnf::transaction;


CPPUNIT_TEST_SUITE_REGISTRATION(TransactionRpmPackageTest);

namespace {

// Allows accessing private methods
create_private_getter_template;
create_getter(new_package, &libdnf::transaction::Transaction::new_package);
create_getter(start, &libdnf::transaction::Transaction::start);
create_getter(finish, &libdnf::transaction::Transaction::finish);
create_getter(new_transaction, &libdnf::transaction::TransactionHistory::new_transaction);

create_getter(set_name, &libdnf::transaction::Package::set_name);
create_getter(set_epoch, &libdnf::transaction::Package::set_epoch);
create_getter(set_version, &libdnf::transaction::Package::set_version);
create_getter(set_release, &libdnf::transaction::Package::set_release);
create_getter(set_arch, &libdnf::transaction::Package::set_arch);
create_getter(set_repoid, &libdnf::transaction::Package::set_repoid);
create_getter(set_action, &libdnf::transaction::Package::set_action);
create_getter(set_reason, &libdnf::transaction::Package::set_reason);
create_getter(set_state, &libdnf::transaction::Package::set_state);

}  //namespace

void TransactionRpmPackageTest::test_save_load() {
    constexpr std::size_t num = 10;

    auto base = new_base();

    // create a new empty transaction
    auto trans = (*(base->get_transaction_history()).*get(new_transaction{}))();

    // create packages in the transaction
    for (std::size_t i = 0; i < num; i++) {
        auto & pkg = (trans.*get(new_package{}))();
        (pkg.*get(set_name{}))("name_" + std::to_string(i));
        (pkg.*get(set_epoch{}))("1");
        (pkg.*get(set_version{}))("2");
        (pkg.*get(set_release{}))("3");
        (pkg.*get(set_arch{}))("x86_64");
        (pkg.*get(set_repoid{}))("repoid");
        (pkg.*get(set_action{}))(TransactionItemAction::INSTALL);
        (pkg.*get(set_reason{}))(TransactionItemReason::USER);
        (pkg.*get(set_state{}))(TransactionItemState::OK);
    }

    // check that there's exactly 10 packages
    CPPUNIT_ASSERT_EQUAL(num, trans.get_packages().size());

    // save the transaction with all transaction items to the database
    (trans.*get(start{}))();
    (trans.*get(finish{}))(TransactionState::OK);

    // create a new Base to force reading the transaction from disk
    auto base2 = new_base();

    // get the written transaction
    auto ts_list = base2->get_transaction_history()->list_transactions({trans.get_id()});
    CPPUNIT_ASSERT_EQUAL(1LU, ts_list.size());

    auto trans2 = ts_list[0];

    // check that there's exactly 10 packages
    CPPUNIT_ASSERT_EQUAL(num, trans2.get_packages().size());

    std::size_t pkg2_num = 0;
    for (auto & pkg2 : trans2.get_packages()) {
        CPPUNIT_ASSERT_EQUAL(std::string("name_") + std::to_string(pkg2_num), pkg2.get_name());
        CPPUNIT_ASSERT_EQUAL(std::string("1"), pkg2.get_epoch());
        CPPUNIT_ASSERT_EQUAL(std::string("2"), pkg2.get_version());
        CPPUNIT_ASSERT_EQUAL(std::string("3"), pkg2.get_release());
        CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), pkg2.get_arch());
        CPPUNIT_ASSERT_EQUAL(std::string("repoid"), pkg2.get_repoid());
        CPPUNIT_ASSERT_EQUAL(TransactionItemAction::INSTALL, pkg2.get_action());
        CPPUNIT_ASSERT_EQUAL(TransactionItemReason::USER, pkg2.get_reason());
        CPPUNIT_ASSERT_EQUAL(TransactionItemState::OK, pkg2.get_state());
        pkg2_num++;
    }
}
