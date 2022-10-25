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


#include "test_comps_environment.hpp"

#include "private_accessor.hpp"

#include "libdnf/comps/group/package.hpp"
#include "libdnf/transaction/comps_environment.hpp"
#include "libdnf/transaction/transaction.hpp"

#include <string>


using namespace libdnf::transaction;


CPPUNIT_TEST_SUITE_REGISTRATION(TransactionCompsEnvironmentTest);

namespace {

// Allows accessing private methods
create_private_getter_template;
create_getter(new_comps_environment, &libdnf::transaction::Transaction::new_comps_environment);
create_getter(start, &libdnf::transaction::Transaction::start);
create_getter(finish, &libdnf::transaction::Transaction::finish);
create_getter(new_transaction, &libdnf::transaction::TransactionHistory::new_transaction);

}  //namespace

CompsEnvironment & create_comps_environment(Transaction & trans) {
    auto & env = (trans.*get(new_comps_environment{}))();

    env.set_environment_id("minimal");
    env.set_name("Minimal Environment");
    env.set_translated_name("translated(Minimal Environment)");
    env.set_package_types(libdnf::comps::PackageType::DEFAULT);

    env.set_repoid("repoid");
    env.set_action(TransactionItemAction::INSTALL);
    env.set_reason(TransactionItemReason::USER);
    env.set_state(TransactionItemState::OK);

    auto & grp_core = env.new_group();
    grp_core.set_group_id("core");
    grp_core.set_installed(true);
    grp_core.set_group_type(libdnf::comps::PackageType::MANDATORY);

    auto & grp_base = env.new_group();
    grp_base.set_group_id("base");
    grp_base.set_installed(false);
    grp_base.set_group_type(libdnf::comps::PackageType::OPTIONAL);

    return env;
}


void TransactionCompsEnvironmentTest::test_save_load() {
    auto base = new_base();

    // create a new empty transaction
    auto trans = (*(base->get_transaction_history()).*get(new_transaction{}))();

    // create an environment in the transaction
    create_comps_environment(trans);

    // check that there's exactly 1 environment
    CPPUNIT_ASSERT_EQUAL(1LU, trans.get_comps_environments().size());

    // save the transaction with all transaction items to the database
    (trans.*get(start{}))();
    (trans.*get(finish{}))(TransactionState::OK);

    // create a new Base to force reading the transaction from disk
    auto base2 = new_base();

    // get the written transaction
    auto ts_list = base2->get_transaction_history()->list_transactions({trans.get_id()});
    CPPUNIT_ASSERT_EQUAL(1LU, ts_list.size());

    auto trans2 = ts_list[0];
    CPPUNIT_ASSERT_EQUAL(1LU, trans2.get_comps_environments().size());

    auto & env2 = trans2.get_comps_environments().at(0);
    CPPUNIT_ASSERT_EQUAL(std::string("minimal"), env2.get_environment_id());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimal Environment"), env2.get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("translated(Minimal Environment)"), env2.get_translated_name());
    CPPUNIT_ASSERT_EQUAL(libdnf::comps::PackageType::DEFAULT, env2.get_package_types());
    CPPUNIT_ASSERT_EQUAL(std::string("repoid"), env2.get_repoid());
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::INSTALL, env2.get_action());
    CPPUNIT_ASSERT_EQUAL(TransactionItemReason::USER, env2.get_reason());
    CPPUNIT_ASSERT_EQUAL(TransactionItemState::OK, env2.get_state());

    // check if the environment has all expected groups in the same order as inserted
    CPPUNIT_ASSERT_EQUAL(2LU, env2.get_groups().size());

    auto & env2_group1 = env2.get_groups().at(0);
    CPPUNIT_ASSERT_EQUAL(std::string("core"), env2_group1.get_group_id());
    CPPUNIT_ASSERT_EQUAL(true, env2_group1.get_installed());
    CPPUNIT_ASSERT_EQUAL(libdnf::comps::PackageType::MANDATORY, env2_group1.get_group_type());

    auto & env2_group2 = env2.get_groups().at(1);
    CPPUNIT_ASSERT_EQUAL(std::string("base"), env2_group2.get_group_id());
    CPPUNIT_ASSERT_EQUAL(false, env2_group2.get_installed());
    CPPUNIT_ASSERT_EQUAL(libdnf::comps::PackageType::OPTIONAL, env2_group2.get_group_type());
}
