#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>

#include "libdnf/transaction/comps_group.hpp"
#include "libdnf/transaction/Transformer.hpp"

#include "CompsGroupItemTest.hpp"
#include "libdnf/transaction/db/db.hpp"
#include "libdnf/transaction/transaction.hpp"


CPPUNIT_TEST_SUITE_REGISTRATION(CompsGroupItemTest);

using namespace libdnf::transaction;

void
CompsGroupItemTest::setUp()
{
    conn = new libdnf::utils::SQLite3(":memory:");
    create_database(*conn);
}

void
CompsGroupItemTest::tearDown()
{
    delete conn;
}

static CompsGroup & create_comps_group(Transaction & trans) {
    auto & grp = trans.new_comps_group();

    //auto grp = std::make_shared< CompsGroup >(trans);
    grp.set_group_id("core");
    grp.set_name("Smallest possible installation");
    grp.set_translated_name("translated(Smallest possible installation)");
    grp.set_package_types(CompsPackageType::DEFAULT);

    grp.set_repoid("");
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

void
CompsGroupItemTest::testCreate()
{
    Transaction trans(*conn);

    auto & grp = create_comps_group(trans);

    trans.begin();
    trans.finish(TransactionState::DONE);

    Transaction trans2(*conn, trans.get_id());

    auto & grp2 = trans2.get_comps_groups().at(0);
    CPPUNIT_ASSERT(grp2->get_id() == grp.get_id());
    CPPUNIT_ASSERT(grp2->get_group_id() == grp.get_group_id());
    CPPUNIT_ASSERT(grp2->get_name() == grp.get_name());
    CPPUNIT_ASSERT(grp2->get_translated_name() == grp.get_translated_name());
    CPPUNIT_ASSERT(grp2->get_package_types() == grp.get_package_types());

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

    create_comps_group(trans);

    trans.begin();
    trans.finish(TransactionState::DONE);

    Transaction trans2(*conn, trans.get_id());

    auto & groups = trans2.get_comps_groups();
    CPPUNIT_ASSERT_EQUAL(1lu, groups.size());

    auto & grp2 = groups.at(0);
    {
        auto & pkg = grp2->get_packages().at(0);
        CPPUNIT_ASSERT_EQUAL(std::string("bash"), pkg->get_name());
    }
    {
        auto & pkg = grp2->get_packages().at(1);
        CPPUNIT_ASSERT_EQUAL(std::string("rpm"), pkg->get_name());
    }
}
