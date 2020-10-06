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


#include "test_rpm_package.hpp"

#include "libdnf/common/sack/query_cmp.hpp"
#include "libdnf/transaction/rpm_package.hpp"
#include "libdnf/transaction/transaction.hpp"

#include <string>


using namespace libdnf::transaction;


CPPUNIT_TEST_SUITE_REGISTRATION(TransactionRpmPackageTest);


void TransactionRpmPackageTest::test_save_load() {
    constexpr std::size_t num = 10;

    auto base = new_base();

    // create a new empty transaction
    auto trans = base->get_transaction_sack().new_transaction();

    // create packages in the transaction
    for (std::size_t i = 0; i < num; i++) {
        auto & pkg = trans->new_package();
        pkg.set_name("name_" + std::to_string(i));
        pkg.set_epoch("1");
        pkg.set_version("2");
        pkg.set_release("3");
        pkg.set_arch("x86_64");
        pkg.set_repoid("repoid");
        pkg.set_action(TransactionItemAction::INSTALL);
        pkg.set_reason(TransactionItemReason::USER);
        pkg.set_state(TransactionItemState::DONE);
    }

    // check that there's exactly 10 packages
    CPPUNIT_ASSERT_EQUAL(num, trans->get_packages().size());

    // save the transaction with all transaction items to the database
    trans->begin();
    trans->finish(TransactionState::DONE);

    // create a new Base to force reading the transaction from disk
    auto base2 = new_base();

    // get the written transaction
    auto q2 = base2->get_transaction_sack().new_query();
    q2.ifilter_id(libdnf::sack::QueryCmp::EXACT, trans->get_id());
    auto trans2 = q2.get();

    // check that there's exactly 10 packages
    CPPUNIT_ASSERT_EQUAL(num, trans2->get_packages().size());

    std::size_t pkg2_num = 0;
    for (auto & pkg2 : trans2->get_packages()) {
        CPPUNIT_ASSERT_EQUAL(std::string("name_") + std::to_string(pkg2_num), pkg2->get_name());
        CPPUNIT_ASSERT_EQUAL(std::string("1"), pkg2->get_epoch());
        CPPUNIT_ASSERT_EQUAL(std::string("2"), pkg2->get_version());
        CPPUNIT_ASSERT_EQUAL(std::string("3"), pkg2->get_release());
        CPPUNIT_ASSERT_EQUAL(std::string("x86_64"), pkg2->get_arch());
        CPPUNIT_ASSERT_EQUAL(std::string("repoid"), pkg2->get_repoid());
        CPPUNIT_ASSERT_EQUAL(TransactionItemAction::INSTALL, pkg2->get_action());
        CPPUNIT_ASSERT_EQUAL(TransactionItemReason::USER, pkg2->get_reason());
        CPPUNIT_ASSERT_EQUAL(TransactionItemState::DONE, pkg2->get_state());
        pkg2_num++;
    }
}
