#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>

#include "libdnf/transaction/comps_environment.hpp"
#include "libdnf/transaction/Transformer.hpp"

#include "CompsEnvironmentItemTest.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(CompsEnvironmentItemTest);

using namespace libdnf::transaction;

void
CompsEnvironmentItemTest::setUp()
{
    conn = new libdnf::utils::SQLite3(":memory:");
    Transformer::createDatabase(*conn);
}

void
CompsEnvironmentItemTest::tearDown()
{
    delete conn;
}

static std::shared_ptr< CompsEnvironment >
createCompsEnvironment(Transaction & trans)
{
    auto env = std::make_shared< CompsEnvironment >(trans);
    env->set_environment_id("minimal");
    env->set_name("Minimal Environment");
    env->set_translated_name("translated(Minimal Environment)");
    env->set_package_types(CompsPackageType::DEFAULT);

    auto & grp_core = env->new_group();
    grp_core.set_group_id("core");
    grp_core.set_installed(true);
    grp_core.set_group_type(CompsPackageType::MANDATORY);

    auto & grp_base = env->new_group();
    grp_base.set_group_id("base");
    grp_base.set_installed(false);
    grp_base.set_group_type(CompsPackageType::OPTIONAL);

    return env;
}

void
CompsEnvironmentItemTest::testCreate()
{
    Transaction trans(*conn);
    auto env = createCompsEnvironment(trans);
    auto ti = trans.addItem(env, "", TransactionItemAction::INSTALL, TransactionItemReason::USER);
    ti->set_state(TransactionItemState::DONE);
    trans.begin();
    trans.finish(TransactionState::DONE);

    CompsEnvironment env2(trans, env->getId());
    CPPUNIT_ASSERT(env2.getId() == env->getId());
    CPPUNIT_ASSERT(env2.get_environment_id() == env->get_environment_id());
    CPPUNIT_ASSERT(env2.get_name() == env->get_name());
    CPPUNIT_ASSERT(env2.get_translated_name() == env->get_translated_name());
    CPPUNIT_ASSERT(env2.get_package_types() == env->get_package_types());

    {
        auto & group = env2.get_groups().at(0);
        CPPUNIT_ASSERT(group->get_group_id() == "base");
        CPPUNIT_ASSERT(group->get_installed() == false);
        CPPUNIT_ASSERT(group->get_group_type() == CompsPackageType::OPTIONAL);
    }
    {
        auto & group = env2.get_groups().at(1);
        CPPUNIT_ASSERT(group->get_group_id() == "core");
        CPPUNIT_ASSERT(group->get_installed() == true);
        CPPUNIT_ASSERT(group->get_group_type() == CompsPackageType::MANDATORY);
    }
}

void
CompsEnvironmentItemTest::testGetTransactionItems()
{
    Transaction trans(*conn);
    auto env = createCompsEnvironment(trans);
    auto ti = trans.addItem(env, "", TransactionItemAction::INSTALL, TransactionItemReason::USER);
    ti->set_state(TransactionItemState::DONE);
    trans.begin();
    trans.finish(TransactionState::DONE);

    Transaction trans2(*conn, trans.get_id());

    auto transItems = trans2.getItems();
    CPPUNIT_ASSERT_EQUAL(1, static_cast< int >(transItems.size()));

    auto transItem = transItems.at(0);

    auto env2 = std::dynamic_pointer_cast<CompsEnvironment>(transItem->getItem());
    {
        auto & group = env2->get_groups().at(0);
        CPPUNIT_ASSERT_EQUAL(std::string("base"), group->get_group_id());
    }
    {
        auto & group = env2->get_groups().at(1);
        CPPUNIT_ASSERT_EQUAL(std::string("core"), group->get_group_id());
    }
}
