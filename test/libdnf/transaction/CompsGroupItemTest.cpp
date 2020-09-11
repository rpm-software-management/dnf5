#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>

#include "libdnf/transaction/CompsGroupItem.hpp"
#include "libdnf/transaction/Transformer.hpp"

#include "CompsGroupItemTest.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(CompsGroupItemTest);

using namespace libdnf::transaction;

void
CompsGroupItemTest::setUp()
{
    conn = std::make_shared< libdnf::utils::SQLite3 >(":memory:");
    Transformer::createDatabase(conn);
}

void
CompsGroupItemTest::tearDown()
{
}

static std::shared_ptr< CompsGroupItem >
createCompsGroup(std::shared_ptr< libdnf::utils::SQLite3 > conn)
{
    auto grp = std::make_shared< CompsGroupItem >(conn);
    grp->setGroupId("core");
    grp->setName("Smallest possible installation");
    grp->setTranslatedName("translated(Smallest possible installation)");
    grp->setPackageTypes(CompsPackageType::DEFAULT);
    grp->addPackage("bash", true, CompsPackageType::MANDATORY);
    grp->addPackage("rpm", false, CompsPackageType::OPTIONAL);
    grp->save();
    return grp;
}

void
CompsGroupItemTest::testCreate()
{
    auto grp = createCompsGroup(conn);

    CompsGroupItem grp2(conn, grp->getId());
    CPPUNIT_ASSERT(grp2.getId() == grp->getId());
    CPPUNIT_ASSERT(grp2.getGroupId() == grp->getGroupId());
    CPPUNIT_ASSERT(grp2.getName() == grp->getName());
    CPPUNIT_ASSERT(grp2.getTranslatedName() == grp->getTranslatedName());
    CPPUNIT_ASSERT(grp2.getPackageTypes() == grp->getPackageTypes());

    {
        auto pkg = grp2.getPackages().at(0);
        CPPUNIT_ASSERT(pkg->getName() == "bash");
        CPPUNIT_ASSERT(pkg->getInstalled() == true);
        CPPUNIT_ASSERT(pkg->getPackageType() == CompsPackageType::MANDATORY);
    }
    {
        auto pkg = grp2.getPackages().at(1);
        CPPUNIT_ASSERT(pkg->getName() == "rpm");
        CPPUNIT_ASSERT(pkg->getInstalled() == false);
        CPPUNIT_ASSERT(pkg->getPackageType() == CompsPackageType::OPTIONAL);
    }

    // test adding a duplicate group
    grp2.addPackage("rpm", true, CompsPackageType::MANDATORY);
    {
        auto pkg = grp2.getPackages().at(1);
        CPPUNIT_ASSERT(pkg->getName() == "rpm");
        CPPUNIT_ASSERT(pkg->getInstalled() == true);
        CPPUNIT_ASSERT(pkg->getPackageType() == CompsPackageType::MANDATORY);
    }
}

void
CompsGroupItemTest::testGetTransactionItems()
{
    Transaction trans(conn);
    auto grp = createCompsGroup(conn);
    auto ti = trans.addItem(grp, "", TransactionItemAction::INSTALL, TransactionItemReason::USER);
    ti->setState(TransactionItemState::DONE);
    trans.begin();
    trans.finish(TransactionState::DONE);

    Transaction trans2(conn, trans.getId());

    auto transItems = trans2.getItems();
    CPPUNIT_ASSERT_EQUAL(1, static_cast< int >(transItems.size()));

    auto transItem = transItems.at(0);

    auto grp2 = transItem->getCompsGroupItem();
    {
        auto pkg = grp2->getPackages().at(0);
        CPPUNIT_ASSERT_EQUAL(std::string("bash"), pkg->getName());
    }
    {
        auto pkg = grp2->getPackages().at(1);
        CPPUNIT_ASSERT_EQUAL(std::string("rpm"), pkg->getName());
    }
}
