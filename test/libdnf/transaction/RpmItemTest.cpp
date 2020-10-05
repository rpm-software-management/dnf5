#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>

#include "libdnf/transaction/rpm_package.hpp"
#include "libdnf/transaction/transaction.hpp"
#include "libdnf/transaction/Transformer.hpp"
#include "libdnf/transaction/db/rpm.hpp"
#include "libdnf/transaction/db/db.hpp"

#include "RpmItemTest.hpp"

using namespace libdnf::transaction;

CPPUNIT_TEST_SUITE_REGISTRATION(RpmItemTest);

void
RpmItemTest::setUp()
{
    conn = new libdnf::utils::SQLite3(":memory:");
    create_database(*conn);
}

void
RpmItemTest::tearDown()
{
    delete conn;
}


/*
DISABLED: adding duplicated was (temporarily?) disabled due to simplification of API

void
RpmItemTest::testCreateDuplicates()
{
    Transaction trans(*conn);

    // bash-4.4.12-5.fc26.x86_64
    auto rpm = std::make_shared< Package >(trans);
    rpm->set_name("bash");
    rpm->set_epoch("0");
    rpm->set_version("4.4.12");
    rpm->set_release("5.fc26");
    rpm->set_arch("x86_64");

    // add a RPM twice, but with different reasons
    auto ti1 = trans.addItem(rpm, "base", TransactionItemAction::INSTALL, TransactionItemReason::GROUP);
    auto ti2 = trans.addItem(rpm, "base", TransactionItemAction::INSTALL, TransactionItemReason::DEPENDENCY);
    // test that the duplicate wasn't inserted
    CPPUNIT_ASSERT(trans.getItems().size() == 1);
    // test that the best reason (from ti1) was used
    CPPUNIT_ASSERT(ti1->get_reason() == TransactionItemReason::GROUP);
    CPPUNIT_ASSERT(ti2->get_reason() == TransactionItemReason::GROUP);

    auto ti3 = trans.addItem(rpm, "base", TransactionItemAction::INSTALL, TransactionItemReason::USER);
    // test that the duplicate wasn't inserted
    CPPUNIT_ASSERT(trans.getItems().size() == 1);
    // test that the best reason (from ti3) was used
    CPPUNIT_ASSERT(ti1->get_reason() == TransactionItemReason::USER);
    CPPUNIT_ASSERT(ti2->get_reason() == TransactionItemReason::USER);
    CPPUNIT_ASSERT(ti3->get_reason() == TransactionItemReason::USER);
}
*/

void
RpmItemTest::testGetTransactionItems()
{
    // performance looks good: 100k records take roughly 3.3s to write, 0.2s to read
    // change following constant to modify number of tested Packages
    constexpr int num = 10;

    Transaction trans(*conn);

    for (int i = 0; i < num; i++) {
        auto & pkg = trans.new_package();
        //auto pkg = std::make_shared< Package >(trans);
        pkg.set_name("name_" + std::to_string(i));
        pkg.set_epoch("0");
        pkg.set_version("1");
        pkg.set_release("2");
        pkg.set_arch("x86_64");
        pkg.set_repoid("base");
        pkg.set_action(TransactionItemAction::INSTALL);
        pkg.set_reason(TransactionItemReason::USER);
        pkg.set_state(TransactionItemState::DONE);
    }
    trans.begin();
    trans.finish(TransactionState::DONE);
    CPPUNIT_ASSERT_EQUAL(1l, trans.get_id());

    Transaction trans2(*conn, trans.get_id());

    auto & packages = trans2.get_packages();
    CPPUNIT_ASSERT_EQUAL(10lu, packages.size());

    auto & pkg2 = packages.at(0);
    CPPUNIT_ASSERT_EQUAL(std::string("name_0"), pkg2->get_name());
}
