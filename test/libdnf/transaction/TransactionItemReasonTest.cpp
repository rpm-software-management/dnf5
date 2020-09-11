#include <cstdio>
#include <iostream>
#include <string>

#include "libdnf/transaction/CompsEnvironmentItem.hpp"
#include "libdnf/transaction/CompsGroupItem.hpp"
#include "libdnf/transaction/RPMItem.hpp"
#include "libdnf/transaction/Swdb.hpp"
#include "libdnf/transaction/Transaction.hpp"
#include "libdnf/transaction/TransactionItem.hpp"
#include "libdnf/transaction/Transformer.hpp"

#include "TransactionItemReasonTest.hpp"

using namespace libdnf::transaction;

CPPUNIT_TEST_SUITE_REGISTRATION(TransactionItemReasonTest);

void
TransactionItemReasonTest::setUp()
{
    conn = std::make_shared< libdnf::utils::SQLite3 >(":memory:");
    Transformer::createDatabase(conn);
}

void
TransactionItemReasonTest::tearDown()
{
}

// no transactions -> UNKNOWN reason
void
TransactionItemReasonTest::testNoTransaction()
{
    Swdb swdb(conn);

    CPPUNIT_ASSERT_EQUAL(static_cast< TransactionItemReason >(
                             swdb.resolveRPMTransactionItemReason("bash", "x86_64", -1)),
                         TransactionItemReason::UNKNOWN);
}

// one transaction with no transaction items -> UNKNOWN reason
void
TransactionItemReasonTest::testEmptyTransaction()
{
    Swdb swdb(conn);

    swdb.initTransaction();
    swdb.beginTransaction(1, "", "", 0);
    swdb.endTransaction(2, "", TransactionState::DONE);
    swdb.closeTransaction();

    CPPUNIT_ASSERT_EQUAL(static_cast< TransactionItemReason >(
                             swdb.resolveRPMTransactionItemReason("bash", "x86_64", -1)),
                         TransactionItemReason::UNKNOWN);
}

// one transaction with one transaction item -> $reason
void
TransactionItemReasonTest::test_OneTransaction_OneTransactionItem()
{
    Swdb swdb(conn);

    swdb.initTransaction();

    auto rpm_bash = std::make_shared< RPMItem >(conn);
    rpm_bash->setName("bash");
    rpm_bash->setEpoch(0);
    rpm_bash->setVersion("4.4.12");
    rpm_bash->setRelease("5.fc26");
    rpm_bash->setArch("x86_64");
    std::string repoid = "base";
    TransactionItemAction action = TransactionItemAction::INSTALL;
    TransactionItemReason reason = TransactionItemReason::GROUP;
    auto ti = swdb.addItem(rpm_bash, repoid, action, reason);
    ti->setState(TransactionItemState::DONE);

    swdb.beginTransaction(1, "", "", 0);
    swdb.endTransaction(2, "", TransactionState::DONE);
    swdb.closeTransaction();

    // package exists -> $reason
    CPPUNIT_ASSERT_EQUAL(static_cast< TransactionItemReason >(
                             swdb.resolveRPMTransactionItemReason("bash", "x86_64", -1)),
                         TransactionItemReason::GROUP);

    // package does not exist -> UNKNOWN
    CPPUNIT_ASSERT_EQUAL(static_cast< TransactionItemReason >(
                             swdb.resolveRPMTransactionItemReason("bash", "i686", -1)),
                         TransactionItemReason::UNKNOWN);

    // package exists, arch not specified -> return best $reason
    CPPUNIT_ASSERT_EQUAL(
        static_cast< TransactionItemReason >(swdb.resolveRPMTransactionItemReason("bash", "", -1)),
        TransactionItemReason::GROUP);
}

// one FAILED transaction with one transaction item -> $reason
void
TransactionItemReasonTest::test_OneFailedTransaction_OneTransactionItem()
{
    Swdb swdb(conn);

    swdb.initTransaction();

    auto rpm_bash = std::make_shared< RPMItem >(conn);
    rpm_bash->setName("bash");
    rpm_bash->setEpoch(0);
    rpm_bash->setVersion("4.4.12");
    rpm_bash->setRelease("5.fc26");
    rpm_bash->setArch("x86_64");
    std::string repoid = "base";
    TransactionItemAction action = TransactionItemAction::INSTALL;
    TransactionItemReason reason = TransactionItemReason::GROUP;
    auto ti = swdb.addItem(rpm_bash, repoid, action, reason);
    ti->setState(TransactionItemState::DONE);

    swdb.beginTransaction(1, "", "", 0);
    swdb.endTransaction(2, "", TransactionState::ERROR);
    swdb.closeTransaction();

    // failed transaction -> UNKNOWN
    CPPUNIT_ASSERT_EQUAL(static_cast< TransactionItemReason >(
                             swdb.resolveRPMTransactionItemReason("bash", "x86_64", -1)),
                         TransactionItemReason::UNKNOWN);

    // failed transaction -> UNKNOWN
    CPPUNIT_ASSERT_EQUAL(static_cast< TransactionItemReason >(
                             swdb.resolveRPMTransactionItemReason("bash", "i686", -1)),
                         TransactionItemReason::UNKNOWN);

    // failed transaction -> UNKNOWN
    CPPUNIT_ASSERT_EQUAL(
        static_cast< TransactionItemReason >(swdb.resolveRPMTransactionItemReason("bash", "", -1)),
        TransactionItemReason::UNKNOWN);
}

// one transaction with two transaction items -> $reason
void
TransactionItemReasonTest::test_OneTransaction_TwoTransactionItems()
{
    Swdb swdb(conn);

    swdb.initTransaction();

    {
        auto rpm_bash = std::make_shared< RPMItem >(conn);
        rpm_bash->setName("bash");
        rpm_bash->setEpoch(0);
        rpm_bash->setVersion("4.4.12");
        rpm_bash->setRelease("5.fc26");
        rpm_bash->setArch("x86_64");
        std::string repoid = "base";
        TransactionItemAction action = TransactionItemAction::INSTALL;
        TransactionItemReason reason = TransactionItemReason::GROUP;
        auto ti = swdb.addItem(rpm_bash, repoid, action, reason);
        ti->setState(TransactionItemState::DONE);
    }

    {
        auto rpm_bash = std::make_shared< RPMItem >(conn);
        rpm_bash->setName("bash");
        rpm_bash->setEpoch(0);
        rpm_bash->setVersion("4.4.12");
        rpm_bash->setRelease("5.fc26");
        rpm_bash->setArch("i686");
        std::string repoid = "base";
        TransactionItemAction action = TransactionItemAction::INSTALL;
        TransactionItemReason reason = TransactionItemReason::USER;
        auto ti = swdb.addItem(rpm_bash, repoid, action, reason);
        ti->setState(TransactionItemState::DONE);
    }

    swdb.beginTransaction(1, "", "", 0);
    swdb.endTransaction(2, "", TransactionState::DONE);
    swdb.closeTransaction();

    // package exists -> $reason
    CPPUNIT_ASSERT_EQUAL(static_cast< TransactionItemReason >(
                             swdb.resolveRPMTransactionItemReason("bash", "x86_64", -1)),
                         TransactionItemReason::GROUP);

    // package exists -> $reason
    CPPUNIT_ASSERT_EQUAL(static_cast< TransactionItemReason >(
                             swdb.resolveRPMTransactionItemReason("bash", "i686", -1)),
                         TransactionItemReason::USER);

    // 2 packages exists, arch not specified -> return best $reason
    CPPUNIT_ASSERT_EQUAL(
        static_cast< TransactionItemReason >(swdb.resolveRPMTransactionItemReason("bash", "", -1)),
        TransactionItemReason::USER);
}

// two transactions with two transaction items -> $reason
void
TransactionItemReasonTest::test_TwoTransactions_TwoTransactionItems()
{
    Swdb swdb(conn);

    {
        swdb.initTransaction();

        auto rpm_bash = std::make_shared< RPMItem >(conn);
        rpm_bash->setName("bash");
        rpm_bash->setEpoch(0);
        rpm_bash->setVersion("4.4.12");
        rpm_bash->setRelease("5.fc26");
        rpm_bash->setArch("x86_64");
        std::string repoid = "base";
        TransactionItemAction action = TransactionItemAction::INSTALL;
        TransactionItemReason reason = TransactionItemReason::GROUP;
        auto ti = swdb.addItem(rpm_bash, repoid, action, reason);
        ti->setState(TransactionItemState::DONE);

        swdb.beginTransaction(1, "", "", 0);
        swdb.endTransaction(2, "", TransactionState::DONE);
        swdb.closeTransaction();
    }

    {
        swdb.initTransaction();

        auto rpm_bash = std::make_shared< RPMItem >(conn);
        rpm_bash->setName("bash");
        rpm_bash->setEpoch(0);
        rpm_bash->setVersion("4.4.12");
        rpm_bash->setRelease("5.fc26");
        rpm_bash->setArch("i686");
        std::string repoid = "base";
        TransactionItemAction action = TransactionItemAction::INSTALL;
        TransactionItemReason reason = TransactionItemReason::USER;
        auto ti = swdb.addItem(rpm_bash, repoid, action, reason);
        ti->setState(TransactionItemState::DONE);

        swdb.beginTransaction(1, "", "", 0);
        swdb.endTransaction(2, "", TransactionState::DONE);
        swdb.closeTransaction();
    }

    // package exists -> $reason
    CPPUNIT_ASSERT_EQUAL(static_cast< TransactionItemReason >(
                             swdb.resolveRPMTransactionItemReason("bash", "x86_64", -1)),
                         TransactionItemReason::GROUP);

    // package exists -> $reason
    CPPUNIT_ASSERT_EQUAL(static_cast< TransactionItemReason >(
                             swdb.resolveRPMTransactionItemReason("bash", "i686", -1)),
                         TransactionItemReason::USER);

    // 2 packages exists, arch not specified -> return best $reason
    CPPUNIT_ASSERT_EQUAL(
        static_cast< TransactionItemReason >(swdb.resolveRPMTransactionItemReason("bash", "", -1)),
        TransactionItemReason::USER);
}

//
void
TransactionItemReasonTest::testRemovedPackage()
{
    Swdb swdb(conn);

    {
        swdb.initTransaction();

        auto rpm_bash = std::make_shared< RPMItem >(conn);
        rpm_bash->setName("bash");
        rpm_bash->setEpoch(0);
        rpm_bash->setVersion("4.4.12");
        rpm_bash->setRelease("5.fc26");
        rpm_bash->setArch("x86_64");
        std::string repoid = "base";
        TransactionItemAction action = TransactionItemAction::INSTALL;
        TransactionItemReason reason = TransactionItemReason::GROUP;
        auto ti = swdb.addItem(rpm_bash, repoid, action, reason);
        ti->setState(TransactionItemState::DONE);

        swdb.beginTransaction(1, "", "", 0);
        swdb.endTransaction(2, "", TransactionState::DONE);
        swdb.closeTransaction();
    }

    {
        swdb.initTransaction();

        auto rpm_bash = std::make_shared< RPMItem >(conn);
        rpm_bash->setName("bash");
        rpm_bash->setEpoch(0);
        rpm_bash->setVersion("4.4.12");
        rpm_bash->setRelease("5.fc26");
        rpm_bash->setArch("i686");
        std::string repoid = "base";
        TransactionItemAction action = TransactionItemAction::INSTALL;
        TransactionItemReason reason = TransactionItemReason::USER;
        auto ti = swdb.addItem(rpm_bash, repoid, action, reason);
        ti->setState(TransactionItemState::DONE);

        swdb.beginTransaction(1, "", "", 0);
        swdb.endTransaction(2, "", TransactionState::DONE);
        swdb.closeTransaction();

        swdb.initTransaction();
        action = TransactionItemAction::REMOVE;
        auto ti_remove = swdb.addItem(rpm_bash, repoid, action, reason);
        ti_remove->setState(TransactionItemState::DONE);
        swdb.beginTransaction(1, "", "", 0);
        swdb.endTransaction(2, "", TransactionState::DONE);
        swdb.closeTransaction();
    }

    // package exists -> $reason
    CPPUNIT_ASSERT_EQUAL(TransactionItemReason::GROUP,
                         static_cast< TransactionItemReason >(
                             swdb.resolveRPMTransactionItemReason("bash", "x86_64", -1)));

    // package exists -> $reason
    CPPUNIT_ASSERT_EQUAL(TransactionItemReason::UNKNOWN,
                         static_cast< TransactionItemReason >(
                             swdb.resolveRPMTransactionItemReason("bash", "i686", -1)));

    // 2 packages exists, arch not specified -> return best $reason
    CPPUNIT_ASSERT_EQUAL(
        TransactionItemReason::GROUP,
        static_cast< TransactionItemReason >(swdb.resolveRPMTransactionItemReason("bash", "", -1)));
}

void
TransactionItemReasonTest::testCompareReasons()
{
    CPPUNIT_ASSERT(TransactionItemReason::USER == TransactionItemReason::USER);
    CPPUNIT_ASSERT(TransactionItemReason::USER <= TransactionItemReason::USER);
    CPPUNIT_ASSERT(TransactionItemReason::USER >= TransactionItemReason::USER);

    CPPUNIT_ASSERT(TransactionItemReason::USER != TransactionItemReason::GROUP);
    CPPUNIT_ASSERT(TransactionItemReason::USER > TransactionItemReason::GROUP);
    CPPUNIT_ASSERT(TransactionItemReason::USER >= TransactionItemReason::GROUP);

    CPPUNIT_ASSERT(TransactionItemReason::GROUP != TransactionItemReason::USER);
    CPPUNIT_ASSERT(TransactionItemReason::GROUP < TransactionItemReason::USER);
    CPPUNIT_ASSERT(TransactionItemReason::GROUP <= TransactionItemReason::USER);
}

void
TransactionItemReasonTest::test_TransactionItemReason_compare()
{
    CPPUNIT_ASSERT_EQUAL(-1, TransactionItemReason_compare(TransactionItemReason::GROUP, TransactionItemReason::USER));
    CPPUNIT_ASSERT_EQUAL(0, TransactionItemReason_compare(TransactionItemReason::USER, TransactionItemReason::USER));
    CPPUNIT_ASSERT_EQUAL(1, TransactionItemReason_compare(TransactionItemReason::USER, TransactionItemReason::GROUP));
}
