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


#include "test_comps_group.hpp"

#include "private_accessor.hpp"

#include "libdnf/comps/group/package.hpp"
#include "libdnf/transaction/comps_group.hpp"
#include "libdnf/transaction/transaction.hpp"

#include <string>


using namespace libdnf::transaction;


CPPUNIT_TEST_SUITE_REGISTRATION(TransactionCompsGroupTest);

namespace {

// Allows accessing private methods
create_private_getter_template;
create_getter(new_comps_group, &libdnf::transaction::Transaction::new_comps_group);
create_getter(start, &libdnf::transaction::Transaction::start);
create_getter(finish, &libdnf::transaction::Transaction::finish);
create_getter(new_transaction, &libdnf::transaction::TransactionHistory::new_transaction);

create_getter(set_group_id, &libdnf::transaction::CompsGroup::set_group_id);
create_getter(set_name, &libdnf::transaction::CompsGroup::set_name);
create_getter(set_translated_name, &libdnf::transaction::CompsGroup::set_translated_name);
create_getter(set_package_types, &libdnf::transaction::CompsGroup::set_package_types);
create_getter(new_package, &libdnf::transaction::CompsGroup::new_package);

create_getter(set_name_pkg, &libdnf::transaction::CompsGroupPackage::set_name);
create_getter(set_installed, &libdnf::transaction::CompsGroupPackage::set_installed);
create_getter(set_package_type, &libdnf::transaction::CompsGroupPackage::set_package_type);

}  // namespace

static CompsGroup & create_comps_group(Transaction & trans) {
    auto & grp = (trans.*get(new_comps_group{}))();

    (grp.*get(set_group_id{}))("core");
    (grp.*get(set_name{}))("Smallest possible installation");
    (grp.*get(set_translated_name{}))("translated(Smallest possible installation)");
    (grp.*get(set_package_types{}))(libdnf::comps::PackageType::DEFAULT);

    grp.set_repoid("repoid");
    grp.set_action(TransactionItemAction::INSTALL);
    grp.set_reason(TransactionItemReason::USER);
    grp.set_state(TransactionItemState::OK);

    auto & pkg1 = (grp.*get(new_package{}))();
    (pkg1.*get(set_name_pkg{}))("bash");
    (pkg1.*get(set_installed{}))(true);
    (pkg1.*get(set_package_type{}))(libdnf::comps::PackageType::MANDATORY);

    auto & pkg2 = (grp.*get(new_package{}))();
    (pkg2.*get(set_name_pkg{}))("rpm");
    (pkg2.*get(set_installed{}))(false);
    (pkg2.*get(set_package_type{}))(libdnf::comps::PackageType::OPTIONAL);

    return grp;
}


void TransactionCompsGroupTest::test_save_load() {
    auto base = new_base();

    //// create a new empty transaction
    auto trans = (*(base->get_transaction_history()).*get(new_transaction{}))();

    //// create an group in the transaction
    create_comps_group(trans);

    //// check that there's exactly 1 group
    //CPPUNIT_ASSERT_EQUAL(1LU, trans.get_comps_groups().size());

    //// save the transaction with all transaction items to the database
    //(trans.*get(start{}))();
    //(trans.*get(finish{}))(TransactionState::OK);

    //// create a new Base to force reading the transaction from disk
    //auto base2 = new_base();

    //// get the written transaction
    //auto ts_list = base2->get_transaction_history()->list_transactions({trans.get_id()});
    //CPPUNIT_ASSERT_EQUAL(1LU, ts_list.size());

    //auto trans2 = ts_list[0];
    //CPPUNIT_ASSERT_EQUAL(1LU, trans2.get_comps_groups().size());

    //auto & grp2 = trans2.get_comps_groups().at(0);
    //CPPUNIT_ASSERT_EQUAL(std::string("core"), grp2.get_group_id());
    //CPPUNIT_ASSERT_EQUAL(std::string("Smallest possible installation"), grp2.get_name());
    //CPPUNIT_ASSERT_EQUAL(std::string("translated(Smallest possible installation)"), grp2.get_translated_name());
    //CPPUNIT_ASSERT_EQUAL(libdnf::comps::PackageType::DEFAULT, grp2.get_package_types());
    //CPPUNIT_ASSERT_EQUAL(std::string("repoid"), grp2.get_repoid());
    //CPPUNIT_ASSERT_EQUAL(TransactionItemAction::INSTALL, grp2.get_action());
    //CPPUNIT_ASSERT_EQUAL(TransactionItemReason::USER, grp2.get_reason());
    //CPPUNIT_ASSERT_EQUAL(TransactionItemState::OK, grp2.get_state());

    //// check if the group has all expected packages in the same order as inserted
    //CPPUNIT_ASSERT_EQUAL(2LU, grp2.get_packages().size());

    //auto & grp2_pkg2 = grp2.get_packages().at(0);
    //CPPUNIT_ASSERT_EQUAL(std::string("bash"), grp2_pkg2.get_name());
    //CPPUNIT_ASSERT_EQUAL(true, grp2_pkg2.get_installed());
    //CPPUNIT_ASSERT_EQUAL(libdnf::comps::PackageType::MANDATORY, grp2_pkg2.get_package_type());

    //auto & grp2_pkg1 = grp2.get_packages().at(1);
    //CPPUNIT_ASSERT_EQUAL(std::string("rpm"), grp2_pkg1.get_name());
    //CPPUNIT_ASSERT_EQUAL(false, grp2_pkg1.get_installed());
    //CPPUNIT_ASSERT_EQUAL(libdnf::comps::PackageType::OPTIONAL, grp2_pkg1.get_package_type());
}
