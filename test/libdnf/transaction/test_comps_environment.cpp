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


#include "test_comps_environment.hpp"

#include "libdnf/common/sack/query_cmp.hpp"
#include "libdnf/transaction/comps_environment.hpp"
#include "libdnf/transaction/transaction.hpp"

#include <string>


using namespace libdnf::transaction;


CPPUNIT_TEST_SUITE_REGISTRATION(TransactionCompsEnvironmentTest);


static CompsEnvironment & create_comps_environment(TransactionWeakPtr & trans) {
    auto & env = trans->new_comps_environment();

    env.set_environment_id("minimal");
    env.set_name("Minimal Environment");
    env.set_translated_name("translated(Minimal Environment)");
    env.set_package_types(CompsPackageType::DEFAULT);

    env.set_repoid("repoid");
    env.set_action(TransactionItemAction::INSTALL);
    env.set_reason(TransactionItemReason::USER);
    env.set_state(TransactionItemState::DONE);

    auto & grp_core = env.new_group();
    grp_core.set_group_id("core");
    grp_core.set_installed(true);
    grp_core.set_group_type(CompsPackageType::MANDATORY);

    auto & grp_base = env.new_group();
    grp_base.set_group_id("base");
    grp_base.set_installed(false);
    grp_base.set_group_type(CompsPackageType::OPTIONAL);

    return env;
}


void TransactionCompsEnvironmentTest::test_save_load() {
    auto base = new_base();

    // create a new empty transaction
    auto trans = base->get_transaction_sack().new_transaction();

    // create an environment in the transaction
    create_comps_environment(trans);

    // check that there's exactly 1 environment
    CPPUNIT_ASSERT_EQUAL(1LU, trans->get_comps_environments().size());

    // save the transaction with all transaction items to the database
    trans->begin();
    trans->finish(TransactionState::DONE);

    // create a new Base to force reading the transaction from disk
    auto base2 = new_base();

    // get the written transaction
    auto q2 = base2->get_transaction_sack().new_query();
    q2.ifilter_id(libdnf::sack::QueryCmp::EXACT, trans->get_id());
    auto trans2 = q2.get();

    // check that there's exactly 1 environment
    CPPUNIT_ASSERT_EQUAL(1LU, trans2->get_comps_environments().size());

    auto & env2 = trans2->get_comps_environments().at(0);
    CPPUNIT_ASSERT_EQUAL(std::string("minimal"), env2->get_environment_id());
    CPPUNIT_ASSERT_EQUAL(std::string("Minimal Environment"), env2->get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("translated(Minimal Environment)"), env2->get_translated_name());
    CPPUNIT_ASSERT_EQUAL(CompsPackageType::DEFAULT, env2->get_package_types());
    CPPUNIT_ASSERT_EQUAL(std::string("repoid"), env2->get_repoid());
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::INSTALL, env2->get_action());
    CPPUNIT_ASSERT_EQUAL(TransactionItemReason::USER, env2->get_reason());
    CPPUNIT_ASSERT_EQUAL(TransactionItemState::DONE, env2->get_state());

    // check if the environment has all expected groups in the same order as inserted
    CPPUNIT_ASSERT_EQUAL(2LU, env2->get_groups().size());

    auto & env2_group1 = env2->get_groups().at(0);
    CPPUNIT_ASSERT_EQUAL(std::string("core"), env2_group1->get_group_id());
    CPPUNIT_ASSERT_EQUAL(true, env2_group1->get_installed());
    CPPUNIT_ASSERT_EQUAL(CompsPackageType::MANDATORY, env2_group1->get_group_type());

    auto & env2_group2 = env2->get_groups().at(1);
    CPPUNIT_ASSERT_EQUAL(std::string("base"), env2_group2->get_group_id());
    CPPUNIT_ASSERT_EQUAL(false, env2_group2->get_installed());
    CPPUNIT_ASSERT_EQUAL(CompsPackageType::OPTIONAL, env2_group2->get_group_type());
}
