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

#include "../shared/private_accessor.hpp"

#include <libdnf5/comps/group/package.hpp>
#include <libdnf5/transaction/comps_environment.hpp>
#include <libdnf5/transaction/transaction.hpp>

#include <string>


using namespace libdnf5::transaction;


CPPUNIT_TEST_SUITE_REGISTRATION(TransactionCompsEnvironmentTest);

namespace {

// Allows accessing private methods
create_private_getter_template;
create_getter(new_comps_environment, &libdnf5::transaction::Transaction::new_comps_environment);
create_getter(start, &libdnf5::transaction::Transaction::start);
create_getter(finish, &libdnf5::transaction::Transaction::finish);
create_getter(new_transaction, &libdnf5::transaction::TransactionHistory::new_transaction);

create_getter(set_environment_id, &libdnf5::transaction::CompsEnvironment::set_environment_id);
create_getter(set_name, &libdnf5::transaction::CompsEnvironment::set_name);
create_getter(set_translated_name, &libdnf5::transaction::CompsEnvironment::set_translated_name);
create_getter(set_package_types, &libdnf5::transaction::CompsEnvironment::set_package_types);
create_getter(set_repoid, &libdnf5::transaction::CompsEnvironment::set_repoid);
create_getter(set_action, &libdnf5::transaction::CompsEnvironment::set_action);
create_getter(set_reason, &libdnf5::transaction::CompsEnvironment::set_reason);
create_getter(set_state, &libdnf5::transaction::CompsEnvironment::set_state);
create_getter(new_group, &libdnf5::transaction::CompsEnvironment::new_group);
create_getter(get_environment_id, &libdnf5::transaction::CompsEnvironment::get_environment_id);
create_getter(get_name, &libdnf5::transaction::CompsEnvironment::get_name);
create_getter(get_translated_name, &libdnf5::transaction::CompsEnvironment::get_translated_name);
create_getter(get_package_types, &libdnf5::transaction::CompsEnvironment::get_package_types);
create_getter(get_repoid, &libdnf5::transaction::CompsEnvironment::get_repoid);
create_getter(get_action, &libdnf5::transaction::CompsEnvironment::get_action);
create_getter(get_reason, &libdnf5::transaction::CompsEnvironment::get_reason);
create_getter(get_state, &libdnf5::transaction::CompsEnvironment::get_state);
create_getter(get_groups, &libdnf5::transaction::CompsEnvironment::get_groups);

create_getter(set_group_id, &libdnf5::transaction::CompsEnvironmentGroup::set_group_id);
create_getter(set_installed, &libdnf5::transaction::CompsEnvironmentGroup::set_installed);
create_getter(set_group_type, &libdnf5::transaction::CompsEnvironmentGroup::set_group_type);
create_getter(get_group_id, &libdnf5::transaction::CompsEnvironmentGroup::get_group_id);
create_getter(get_installed, &libdnf5::transaction::CompsEnvironmentGroup::get_installed);
create_getter(get_group_type, &libdnf5::transaction::CompsEnvironmentGroup::get_group_type);

}  //namespace

CompsEnvironment & create_comps_environment(Transaction & trans) {
    auto & env = (trans.*get(new_comps_environment{}))();

    (env.*get(set_environment_id{}))("minimal");
    (env.*get(set_name{}))("Minimal Environment");
    (env.*get(set_translated_name{}))("translated(Minimal Environment)");
    (env.*get(set_package_types{}))(libdnf5::comps::PackageType::DEFAULT);

    (env.*get(set_repoid{}))("repoid");
    (env.*get(set_action{}))(TransactionItemAction::INSTALL);
    (env.*get(set_reason{}))(TransactionItemReason::USER);
    (env.*get(set_state{}))(TransactionItemState::OK);

    auto & grp_core = (env.*get(new_group{}))();
    (grp_core.*get(set_group_id{}))("core");
    (grp_core.*get(set_installed{}))(true);
    (grp_core.*get(set_group_type{}))(libdnf5::comps::PackageType::MANDATORY);

    auto & grp_base = (env.*get(new_group{}))();
    (grp_base.*get(set_group_id{}))("base");
    (grp_base.*get(set_installed{}))(false);
    (grp_base.*get(set_group_type{}))(libdnf5::comps::PackageType::OPTIONAL);

    return env;
}


void TransactionCompsEnvironmentTest::test_save_load() {
    auto base = new_base();

    // create a new empty transaction
    libdnf5::transaction::TransactionHistory history(base->get_weak_ptr());
    auto trans = (history.*get(new_transaction{}))();

    // create an environment in the transaction
    create_comps_environment(trans);

    // check that there's exactly 1 environment
    CPPUNIT_ASSERT_EQUAL((size_t)1, trans.get_comps_environments().size());

    // save the transaction with all transaction items to the database
    (trans.*get(start{}))();
    (trans.*get(finish{}))(TransactionState::OK);

    // create a new Base to force reading the transaction from disk
    auto base2 = new_base();

    // get the written transaction
    libdnf5::transaction::TransactionHistory history2(base2->get_weak_ptr());
    auto ts_list = history2.list_transactions({trans.get_id()});
    CPPUNIT_ASSERT_EQUAL((size_t)1, ts_list.size());

    auto trans2 = ts_list[0];
    CPPUNIT_ASSERT_EQUAL((size_t)1, trans2.get_comps_environments().size());

    auto & env2 = trans2.get_comps_environments().at(0);
    CPPUNIT_ASSERT_EQUAL(std::string("minimal"), (env2.*get(get_environment_id{}))());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimal Environment"), (env2.*get(get_name{}))());
    CPPUNIT_ASSERT_EQUAL(std::string("translated(Minimal Environment)"), (env2.*get(get_translated_name{}))());
    CPPUNIT_ASSERT_EQUAL(libdnf5::comps::PackageType::DEFAULT, (env2.*get(get_package_types{}))());
    CPPUNIT_ASSERT_EQUAL(std::string("repoid"), (env2.*get(get_repoid{}))());
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::INSTALL, (env2.*get(get_action{}))());
    CPPUNIT_ASSERT_EQUAL(TransactionItemReason::USER, (env2.*get(get_reason{}))());
    CPPUNIT_ASSERT_EQUAL(TransactionItemState::OK, (env2.*get(get_state{}))());

    // check if the environment has all expected groups in the same order as inserted
    CPPUNIT_ASSERT_EQUAL((size_t)2, (env2.*get(get_groups{}))().size());

    auto & env2_group1 = (env2.*get(get_groups{}))().at(0);
    CPPUNIT_ASSERT_EQUAL(std::string("core"), (env2_group1.*get(get_group_id{}))());
    CPPUNIT_ASSERT_EQUAL(true, (env2_group1.*get(get_installed{}))());
    CPPUNIT_ASSERT_EQUAL(libdnf5::comps::PackageType::MANDATORY, (env2_group1.*get(get_group_type{}))());

    auto & env2_group2 = (env2.*get(get_groups{}))().at(1);
    CPPUNIT_ASSERT_EQUAL(std::string("base"), (env2_group2.*get(get_group_id{}))());
    CPPUNIT_ASSERT_EQUAL(false, (env2_group2.*get(get_installed{}))());
    CPPUNIT_ASSERT_EQUAL(libdnf5::comps::PackageType::OPTIONAL, (env2_group2.*get(get_group_type{}))());
}
