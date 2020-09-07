#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>

#include "../backports.hpp"

#include "libdnf/transaction/CompsEnvironmentItem.hpp"
#include "libdnf/transaction/Transformer.hpp"

#include "CompsEnvironmentItemTest.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(CompsEnvironmentItemTest);

using namespace libdnf;

void
CompsEnvironmentItemTest::setUp()
{
    conn = std::make_shared< SQLite3 >(":memory:");
    Transformer::createDatabase(conn);
}

void
CompsEnvironmentItemTest::tearDown()
{
}

static std::shared_ptr< CompsEnvironmentItem >
createCompsEnvironment(std::shared_ptr< SQLite3 > conn)
{
    auto env = std::make_shared< CompsEnvironmentItem >(conn);
    env->setEnvironmentId("minimal");
    env->setName("Minimal Environment");
    env->setTranslatedName("translated(Minimal Environment)");
    env->setPackageTypes(CompsPackageType::DEFAULT);
    env->addGroup("core", true, CompsPackageType::MANDATORY);
    env->addGroup("base", false, CompsPackageType::OPTIONAL);
    env->save();
    return env;
}

void
CompsEnvironmentItemTest::testCreate()
{
    auto env = createCompsEnvironment(conn);

    CompsEnvironmentItem env2(conn, env->getId());
    CPPUNIT_ASSERT(env2.getId() == env->getId());
    CPPUNIT_ASSERT(env2.getEnvironmentId() == env->getEnvironmentId());
    CPPUNIT_ASSERT(env2.getName() == env->getName());
    CPPUNIT_ASSERT(env2.getTranslatedName() == env->getTranslatedName());
    CPPUNIT_ASSERT(env2.getPackageTypes() == env->getPackageTypes());

    {
        auto group = env2.getGroups().at(0);
        CPPUNIT_ASSERT(group->getGroupId() == "base");
        CPPUNIT_ASSERT(group->getInstalled() == false);
        CPPUNIT_ASSERT(group->getGroupType() == CompsPackageType::OPTIONAL);
    }
    {
        auto group = env2.getGroups().at(1);
        CPPUNIT_ASSERT(group->getGroupId() == "core");
        CPPUNIT_ASSERT(group->getInstalled() == true);
        CPPUNIT_ASSERT(group->getGroupType() == CompsPackageType::MANDATORY);
    }

    // test adding a duplicate group
    env2.addGroup("base", true, CompsPackageType::MANDATORY);
    {
        auto group = env2.getGroups().at(0);
        CPPUNIT_ASSERT(group->getGroupId() == "base");
        CPPUNIT_ASSERT(group->getInstalled() == true);
        CPPUNIT_ASSERT(group->getGroupType() == CompsPackageType::MANDATORY);
    }
}

void
CompsEnvironmentItemTest::testGetTransactionItems()
{
    libdnf::swdb_private::Transaction trans(conn);
    auto env = createCompsEnvironment(conn);
    auto ti = trans.addItem(env, "", TransactionItemAction::INSTALL, TransactionItemReason::USER);
    ti->setState(TransactionItemState::DONE);
    trans.begin();
    trans.finish(TransactionState::DONE);

    libdnf::Transaction trans2(conn, trans.getId());

    auto transItems = trans2.getItems();
    CPPUNIT_ASSERT_EQUAL(1, static_cast< int >(transItems.size()));

    auto transItem = transItems.at(0);

    auto env2 = transItem->getCompsEnvironmentItem();
    {
        auto group = env2->getGroups().at(0);
        CPPUNIT_ASSERT_EQUAL(std::string("base"), group->getGroupId());
    }
    {
        auto group = env2->getGroups().at(1);
        CPPUNIT_ASSERT_EQUAL(std::string("core"), group->getGroupId());
    }
}
