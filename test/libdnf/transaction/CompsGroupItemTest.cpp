#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>

#include "libdnf/transaction/comps_group.hpp"
#include "libdnf/transaction/Transformer.hpp"

#include "CompsGroupItemTest.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(CompsGroupItemTest);

using namespace libdnf::transaction;

void
CompsGroupItemTest::setUp()
{
    conn = new libdnf::utils::SQLite3(":memory:");
    Transformer::createDatabase(*conn);
}

void
CompsGroupItemTest::tearDown()
{
    delete conn;
}

static std::shared_ptr< CompsGroup >
createCompsGroup(Transaction & trans)
{
    auto grp = std::make_shared< CompsGroup >(trans);
    grp->set_group_id("core");
    grp->set_name("Smallest possible installation");
    grp->set_translated_name("translated(Smallest possible installation)");
    grp->set_package_types(CompsPackageType::DEFAULT);

    auto & pkg1 = grp->new_package();
    pkg1.set_name("bash");
    pkg1.set_installed(true);
    pkg1.set_package_type(CompsPackageType::MANDATORY);

    auto & pkg2 = grp->new_package();
    pkg2.set_name("rpm");
    pkg2.set_installed(false);
    pkg2.set_package_type(CompsPackageType::OPTIONAL);

    return grp;
}

void
CompsGroupItemTest::testCreate()
{
    Transaction trans(*conn);
    auto grp = createCompsGroup(trans);
    auto item = trans.addItem(grp, "", TransactionItemAction::INSTALL, TransactionItemReason::USER);
    item->set_state(TransactionItemState::DONE);
    trans.begin();
    trans.finish(TransactionState::DONE);

    Transaction trans2(*conn, trans.get_id());

    auto grp2 = std::dynamic_pointer_cast<CompsGroup>(trans.getItems().at(0)->getItem());
    CPPUNIT_ASSERT(grp2->getId() == grp->getId());
    CPPUNIT_ASSERT(grp2->get_group_id() == grp->get_group_id());
    CPPUNIT_ASSERT(grp2->get_name() == grp->get_name());
    CPPUNIT_ASSERT(grp2->get_translated_name() == grp->get_translated_name());
    CPPUNIT_ASSERT(grp2->get_package_types() == grp->get_package_types());

    {
        auto & pkg = grp2->get_packages().at(0);
        CPPUNIT_ASSERT(pkg->get_name() == "bash");
        CPPUNIT_ASSERT(pkg->get_installed() == true);
        CPPUNIT_ASSERT(pkg->get_package_type() == CompsPackageType::MANDATORY);
    }
    {
        auto & pkg = grp2->get_packages().at(1);
        CPPUNIT_ASSERT(pkg->get_name() == "rpm");
        CPPUNIT_ASSERT(pkg->get_installed() == false);
        CPPUNIT_ASSERT(pkg->get_package_type() == CompsPackageType::OPTIONAL);
    }
}

void
CompsGroupItemTest::testGetTransactionItems()
{
    Transaction trans(*conn);
    auto grp = createCompsGroup(trans);
    auto ti = trans.addItem(grp, "", TransactionItemAction::INSTALL, TransactionItemReason::USER);
    ti->set_state(TransactionItemState::DONE);
    trans.begin();
    trans.finish(TransactionState::DONE);

    Transaction trans2(*conn, trans.get_id());

    auto transItems = trans2.getItems();
    CPPUNIT_ASSERT_EQUAL(1, static_cast< int >(transItems.size()));

    auto transItem = transItems.at(0);

    auto grp2 = std::dynamic_pointer_cast<CompsGroup>(transItem->getItem());
    {
        auto & pkg = grp2->get_packages().at(0);
        CPPUNIT_ASSERT_EQUAL(std::string("bash"), pkg->get_name());
    }
    {
        auto & pkg = grp2->get_packages().at(1);
        CPPUNIT_ASSERT_EQUAL(std::string("rpm"), pkg->get_name());
    }
}
