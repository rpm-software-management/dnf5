#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>

#include "libdnf/transaction/comps_environment.hpp"
#include "libdnf/transaction/Transformer.hpp"

#include "CompsEnvironmentItemTest.hpp"
#include "libdnf/transaction/db/db.hpp"
#include "libdnf/transaction/transaction.hpp"


CPPUNIT_TEST_SUITE_REGISTRATION(CompsEnvironmentItemTest);

using namespace libdnf::transaction;

void
CompsEnvironmentItemTest::setUp()
{
    conn = new libdnf::utils::SQLite3(":memory:");
    create_database(*conn);
}

void
CompsEnvironmentItemTest::tearDown()
{
    delete conn;
}

static CompsEnvironment & create_comps_environment(Transaction & trans) {
    auto & env = trans.new_comps_environment();

    env.set_environment_id("minimal");
    env.set_name("Minimal Environment");
    env.set_translated_name("translated(Minimal Environment)");
    env.set_package_types(CompsPackageType::DEFAULT);

    env.set_repoid("");
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

void
CompsEnvironmentItemTest::testCreate()
{
    Transaction trans(*conn);

    auto & env = create_comps_environment(trans);

    trans.begin();
    trans.finish(TransactionState::DONE);

    Transaction trans2(*conn, trans.get_id());

    auto & env2 = trans.get_comps_environments().at(0);

    CPPUNIT_ASSERT(env2->get_id() == env.get_id());
    CPPUNIT_ASSERT(env2->get_environment_id() == env.get_environment_id());
    CPPUNIT_ASSERT(env2->get_name() == env.get_name());
    CPPUNIT_ASSERT(env2->get_translated_name() == env.get_translated_name());
    CPPUNIT_ASSERT(env2->get_package_types() == env.get_package_types());

    {
        auto & group = env2->get_groups().at(0);
        CPPUNIT_ASSERT(group->get_group_id() == "core");
        CPPUNIT_ASSERT(group->get_installed() == true);
        CPPUNIT_ASSERT(group->get_group_type() == CompsPackageType::MANDATORY);
    }
    {
        auto & group = env2->get_groups().at(1);
        CPPUNIT_ASSERT(group->get_group_id() == "base");
        CPPUNIT_ASSERT(group->get_installed() == false);
        CPPUNIT_ASSERT(group->get_group_type() == CompsPackageType::OPTIONAL);
    }
}

void
CompsEnvironmentItemTest::testGetTransactionItems()
{
    Transaction trans(*conn);

    create_comps_environment(trans);
    CPPUNIT_ASSERT_EQUAL(1lu, trans.get_comps_environments().size());

    trans.begin();
    trans.finish(TransactionState::DONE);

    Transaction trans2(*conn, trans.get_id());

    auto & environments = trans2.get_comps_environments();
    CPPUNIT_ASSERT_EQUAL(1lu, environments.size());

    auto & env2 = environments.at(0);
    {
        auto & group = env2->get_groups().at(0);
        CPPUNIT_ASSERT_EQUAL(std::string("base"), group->get_group_id());
    }
    {
        auto & group = env2->get_groups().at(1);
        CPPUNIT_ASSERT_EQUAL(std::string("core"), group->get_group_id());
    }
}
