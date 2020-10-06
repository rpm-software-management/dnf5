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


#include "test_comps_group.hpp"

#include "libdnf/common/sack/query_cmp.hpp"
#include "libdnf/transaction/comps_group.hpp"
#include "libdnf/transaction/transaction.hpp"

#include <string>


using namespace libdnf::transaction;


CPPUNIT_TEST_SUITE_REGISTRATION(TransactionCompsGroupTest);


static CompsGroup & create_comps_group(TransactionWeakPtr & trans) {
    auto & grp = trans->new_comps_group();

    grp.set_group_id("core");
    grp.set_name("Smallest possible installation");
    grp.set_translated_name("translated(Smallest possible installation)");
    grp.set_package_types(CompsPackageType::DEFAULT);

    grp.set_repoid("repoid");
    grp.set_action(TransactionItemAction::INSTALL);
    grp.set_reason(TransactionItemReason::USER);
    grp.set_state(TransactionItemState::DONE);

    auto & pkg1 = grp.new_package();
    pkg1.set_name("bash");
    pkg1.set_installed(true);
    pkg1.set_package_type(CompsPackageType::MANDATORY);

    auto & pkg2 = grp.new_package();
    pkg2.set_name("rpm");
    pkg2.set_installed(false);
    pkg2.set_package_type(CompsPackageType::OPTIONAL);

    return grp;
}


void TransactionCompsGroupTest::test_save_load() {
    auto base = new_base();

    // create a new empty transaction
    auto trans = base->get_transaction_sack().new_transaction();

    // create an group in the transaction
    create_comps_group(trans);

    // check that there's exactly 1 group
    CPPUNIT_ASSERT_EQUAL(1LU, trans->get_comps_groups().size());

    // save the transaction with all transaction items to the database
    trans->begin();
    trans->finish(TransactionState::DONE);

    // create a new Base to force reading the transaction from disk
    auto base2 = new_base();

    // get the written transaction
    auto q2 = base2->get_transaction_sack().new_query();
    q2.ifilter_id(libdnf::sack::QueryCmp::EXACT, trans->get_id());
    auto trans2 = q2.get();

    // check that there's exactly 1 group
    CPPUNIT_ASSERT_EQUAL(1LU, trans2->get_comps_groups().size());

    auto & grp2 = trans2->get_comps_groups().at(0);
    CPPUNIT_ASSERT_EQUAL(std::string("core"), grp2->get_group_id());
    CPPUNIT_ASSERT_EQUAL(std::string("Smallest possible installation"), grp2->get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("translated(Smallest possible installation)"), grp2->get_translated_name());
    CPPUNIT_ASSERT_EQUAL(CompsPackageType::DEFAULT, grp2->get_package_types());
    CPPUNIT_ASSERT_EQUAL(std::string("repoid"), grp2->get_repoid());
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::INSTALL, grp2->get_action());
    CPPUNIT_ASSERT_EQUAL(TransactionItemReason::USER, grp2->get_reason());
    CPPUNIT_ASSERT_EQUAL(TransactionItemState::DONE, grp2->get_state());

    // check if the group has all expected packages in the same order as inserted
    CPPUNIT_ASSERT_EQUAL(2LU, grp2->get_packages().size());

    auto & grp2_pkg2 = grp2->get_packages().at(0);
    CPPUNIT_ASSERT_EQUAL(std::string("bash"), grp2_pkg2->get_name());
    CPPUNIT_ASSERT_EQUAL(true, grp2_pkg2->get_installed());
    CPPUNIT_ASSERT_EQUAL(CompsPackageType::MANDATORY, grp2_pkg2->get_package_type());

    auto & grp2_pkg1 = grp2->get_packages().at(1);
    CPPUNIT_ASSERT_EQUAL(std::string("rpm"), grp2_pkg1->get_name());
    CPPUNIT_ASSERT_EQUAL(false, grp2_pkg1->get_installed());
    CPPUNIT_ASSERT_EQUAL(CompsPackageType::OPTIONAL, grp2_pkg1->get_package_type());
}
