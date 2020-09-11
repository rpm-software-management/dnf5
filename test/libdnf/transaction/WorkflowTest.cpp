#include <cstdio>
#include <iostream>
#include <string>

#include "libdnf/transaction/CompsEnvironmentItem.hpp"
#include "libdnf/transaction/CompsGroupItem.hpp"
#include "libdnf/transaction/RPMItem.hpp"
#include "libdnf/transaction/Transaction.hpp"
#include "libdnf/transaction/TransactionItem.hpp"
#include "libdnf/transaction/Transformer.hpp"
#include "libdnf/transaction/Types.hpp"

#include "WorkflowTest.hpp"

using namespace libdnf::transaction;

CPPUNIT_TEST_SUITE_REGISTRATION(WorkflowTest);

void
WorkflowTest::setUp()
{
    conn = std::make_shared< libdnf::utils::SQLite3 >(":memory:");
    Transformer::createDatabase(conn);
}

void
WorkflowTest::tearDown()
{
}

void
WorkflowTest::testDefaultWorkflow()
{
    // TODO: init/begin/end trans

    // STEP 1: create transaction object
    Transaction trans(conn);
    CPPUNIT_ASSERT_EQUAL(TransactionState::UNKNOWN, trans.getState());

    // STEP 2: set vars
    trans.setReleasever("26");

    // populate goal
    // resolve dependencies
    // prepare RPM transaction

    // STEP 3: associate RPMs to the transaction
    // bash-4.4.12-5.fc26.x86_64
    auto rpm_bash = std::make_shared< RPMItem >(conn);
    rpm_bash->setName("bash");
    rpm_bash->setEpoch(0);
    rpm_bash->setVersion("4.4.12");
    rpm_bash->setRelease("5.fc26");
    rpm_bash->setArch("x86_64");
    std::string repoid = "base";
    TransactionItemAction action = TransactionItemAction::INSTALL;
    TransactionItemReason reason = TransactionItemReason::GROUP;
    trans.addItem(rpm_bash, repoid, action, reason);

    // systemd-233-6.fc26
    auto rpm_systemd = std::make_shared< RPMItem >(conn);
    rpm_systemd->setName("systemd");
    rpm_systemd->setEpoch(0);
    rpm_systemd->setVersion("233");
    rpm_systemd->setRelease("6.fc26");
    rpm_systemd->setArch("x86_64");
    repoid = "base";
    action = TransactionItemAction::OBSOLETE;
    reason = TransactionItemReason::USER;
    auto ti_rpm_systemd = trans.addItem(rpm_systemd, repoid, action, reason);

    // sysvinit-2.88-14.dsf.fc20
    auto rpm_sysvinit = std::make_shared< RPMItem >(conn);
    rpm_sysvinit->setName("sysvinit");
    rpm_sysvinit->setEpoch(0);
    rpm_sysvinit->setVersion("2.88");
    rpm_sysvinit->setRelease("14.dsf.fc20");
    rpm_sysvinit->setArch("x86_64");
    repoid = "f20";
    action = TransactionItemAction::OBSOLETED;
    reason = TransactionItemReason::USER;
    auto ti_rpm_sysvinit = trans.addItem(rpm_sysvinit, repoid, action, reason);
    ti_rpm_sysvinit->addReplacedBy(ti_rpm_systemd);

    auto comps_group_core = std::make_shared< CompsGroupItem >(conn);
    comps_group_core->setGroupId("core");
    comps_group_core->setName("Core");
    comps_group_core->setTranslatedName("Úplný základ");
    comps_group_core->addPackage("bash", true, CompsPackageType::MANDATORY);
    repoid = "";
    action = TransactionItemAction::INSTALL;
    reason = TransactionItemReason::USER;
    trans.addItem(comps_group_core, repoid, action, reason);

    auto comps_environment_minimal = std::make_shared< CompsEnvironmentItem >(conn);
    comps_environment_minimal->setEnvironmentId("minimal");
    comps_environment_minimal->setName("Minimal");
    comps_environment_minimal->setTranslatedName("mmm");
    comps_environment_minimal->addGroup("core", true, CompsPackageType::MANDATORY);
    repoid = "";
    action = TransactionItemAction::INSTALL;
    reason = TransactionItemReason::USER;
    trans.addItem(comps_environment_minimal, repoid, action, reason);

    // STEP 4: save transaction and all associated items
    trans.begin();

    // STEP 5: run RPM transaction; callback: mark completed items
    for (auto i : trans.getItems()) {
        i->setState(TransactionItemState::DONE);
        i->save();
    }

    // STEP 6
    // mark completed transaction
    trans.finish(TransactionState::DONE);
    CPPUNIT_ASSERT_EQUAL(TransactionState::DONE, trans.getState());

    // VERIFY
    // verify that data is available via public API
    auto trans2 = Transaction(conn, trans.getId());
    CPPUNIT_ASSERT_EQUAL(TransactionState::DONE, trans2.getState());

    CPPUNIT_ASSERT(trans2.getItems().size() == 5);

    for (auto i : trans2.getItems()) {
        if (i->getId() == 1) {
            CPPUNIT_ASSERT(i->getAction() == TransactionItemAction::INSTALL);
            CPPUNIT_ASSERT(i->getReason() == TransactionItemReason::GROUP);
            CPPUNIT_ASSERT(i->getRepoid() == "base");
        } else if (i->getId() == 2) {
            CPPUNIT_ASSERT(i->getAction() == TransactionItemAction::OBSOLETE);
            CPPUNIT_ASSERT(i->getReason() == TransactionItemReason::USER);
            CPPUNIT_ASSERT(i->getRepoid() == "base");
        } else if (i->getId() == 3) {
            CPPUNIT_ASSERT(i->getAction() == TransactionItemAction::OBSOLETED);
            CPPUNIT_ASSERT(i->getReason() == TransactionItemReason::USER);
            CPPUNIT_ASSERT(i->getRepoid() == "f20");
        }

        // CPPUNIT_ASSERT(i->getItem()->getItemType() == "rpm");
        CPPUNIT_ASSERT_EQUAL(TransactionItemState::DONE, i->getState());
        // std::cout << "TransactionItem: " << i->getItem()->toStr() << std::endl;
        if (i->getItem()->getItemType() == ItemType::GROUP) {
            auto grp = std::dynamic_pointer_cast< CompsGroupItem >(i->getItem());
            CPPUNIT_ASSERT(grp->getPackages().size() == 1);
            for (auto i : grp->getPackages()) {
                // std::cout << "  CompsGroupPackage: " << i->getName() << std::endl;
            }
        }
        if (i->getItem()->getItemType() == ItemType::ENVIRONMENT) {
            auto env = std::dynamic_pointer_cast< CompsEnvironmentItem >(i->getItem());
            CPPUNIT_ASSERT(env->getGroups().size() == 1);
            for (auto i : env->getGroups()) {
                // std::cout << "  CompsEnvironmentGroup: @" << i->getGroupId() << std::endl;
            }
        }
    }
}
