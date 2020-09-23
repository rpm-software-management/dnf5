#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>

#include "libdnf/transaction/rpm_package.hpp"
#include "libdnf/transaction/Transformer.hpp"
#include "libdnf/transaction/db/rpm.hpp"

#include "RpmItemTest.hpp"

using namespace libdnf::transaction;

CPPUNIT_TEST_SUITE_REGISTRATION(RpmItemTest);

void
RpmItemTest::setUp()
{
    conn = new libdnf::utils::SQLite3(":memory:");
    Transformer::createDatabase(*conn);
}

void
RpmItemTest::tearDown()
{
    delete conn;
}

void
RpmItemTest::testCreate()
{
    Transaction trans(*conn);
    // bash-4.4.12-5.fc26.x86_64
    Package rpm(trans);
    rpm.set_name("bash");
    rpm.set_epoch(0);
    rpm.set_version("4.4.12");
    rpm.set_release("5.fc26");
    rpm.set_arch("x86_64");
    rpm.save();

    Package rpm2(trans);
    auto q_select = rpm_select_new_query(*conn);
    rpm_select(*q_select, rpm.getId(), rpm2);
    CPPUNIT_ASSERT(rpm2.getId() == rpm.getId());
    CPPUNIT_ASSERT(rpm2.get_name() == rpm.get_name());
    CPPUNIT_ASSERT(rpm2.get_epoch() == rpm.get_epoch());
    CPPUNIT_ASSERT(rpm2.get_version() == rpm.get_version());
    CPPUNIT_ASSERT(rpm2.get_release() == rpm.get_release());
}

void
RpmItemTest::testCreateDuplicates()
{
    Transaction trans(*conn);

    // bash-4.4.12-5.fc26.x86_64
    auto rpm = std::make_shared< Package >(trans);
    rpm->set_name("bash");
    rpm->set_epoch(0);
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

void
RpmItemTest::testGetTransactionItems()
{
    // performance looks good: 100k records take roughly 3.3s to write, 0.2s to read
    // change following constant to modify number of tested Packages
    constexpr int num = 10;

    Transaction trans(*conn);

    auto create_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num; i++) {
        auto rpm = std::make_shared< Package >(trans);
        rpm->set_name("name_" + std::to_string(i));
        rpm->set_epoch(0);
        rpm->set_version("1");
        rpm->set_release("2");
        rpm->set_arch("x86_64");
        auto ti = trans.addItem(rpm, "base", TransactionItemAction::INSTALL, TransactionItemReason::USER);
        ti->set_state(TransactionItemState::DONE);
    }
    trans.begin();
    trans.finish(TransactionState::DONE);
    auto create_finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration< double > create_duration = create_finish - create_start;

    auto read_start = std::chrono::high_resolution_clock::now();
    auto items = get_transaction_packages(trans);
    for (auto i : items) {
        auto rpm = std::dynamic_pointer_cast< Package >(i->getItem());
        // std::cout << rpm->getNEVRA() << std::endl;
    }
    auto read_finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration< double > read_duration = read_finish - read_start;

    auto createMs = std::chrono::duration_cast< std::chrono::milliseconds >(create_duration);
    auto readMs = std::chrono::duration_cast< std::chrono::milliseconds >(read_duration);

    //CPPUNIT_ASSERT(createMs.count() == 0);
    //CPPUNIT_ASSERT(readMs.count() == 0);
}
