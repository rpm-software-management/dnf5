#include <cstdio>
#include <iostream>
#include <string>

#include "libdnf/transaction/comps_environment.hpp"
#include "libdnf/transaction/comps_group.hpp"
#include "libdnf/transaction/rpm_package.hpp"
#include "libdnf/transaction/transaction.hpp"
#include "libdnf/transaction/transaction_item.hpp"
#include "libdnf/transaction/Transformer.hpp"
#include "libdnf/transaction/db/db.hpp"

#include "WorkflowTest.hpp"

using namespace libdnf::transaction;

CPPUNIT_TEST_SUITE_REGISTRATION(WorkflowTest);

void
WorkflowTest::setUp()
{
    conn = new libdnf::utils::SQLite3(":memory:");
    create_database(*conn);
}

void
WorkflowTest::tearDown()
{
    delete conn;
}

void
WorkflowTest::testDefaultWorkflow()
{
    // TODO: init/begin/end trans

    // STEP 1: create transaction object
    Transaction trans(*conn);
    CPPUNIT_ASSERT_EQUAL(TransactionState::UNKNOWN, trans.get_state());

    // STEP 2: set vars
    trans.set_releasever("26");

    // populate goal
    // resolve dependencies
    // prepare RPM transaction

    // STEP 3: associate RPMs to the transaction
    // bash-4.4.12-5.fc26.x86_64
    auto & rpm_bash = trans.new_package();
    rpm_bash.set_name("bash");
    rpm_bash.set_epoch("0");
    rpm_bash.set_version("4.4.12");
    rpm_bash.set_release("5.fc26");
    rpm_bash.set_arch("x86_64");
    rpm_bash.set_repoid("base");
    rpm_bash.set_action(TransactionItemAction::INSTALL);
    rpm_bash.set_reason(TransactionItemReason::GROUP);

    // systemd-233-6.fc26
    auto & rpm_systemd = trans.new_package();
    rpm_systemd.set_name("systemd");
    rpm_systemd.set_epoch("0");
    rpm_systemd.set_version("233");
    rpm_systemd.set_release("6.fc26");
    rpm_systemd.set_arch("x86_64");
    rpm_systemd.set_repoid("base");
    rpm_systemd.set_action(TransactionItemAction::OBSOLETE);
    rpm_systemd.set_reason(TransactionItemReason::USER);

    // sysvinit-2.88-14.dsf.fc20
    auto & rpm_sysvinit = trans.new_package();
    rpm_sysvinit.set_name("sysvinit");
    rpm_sysvinit.set_epoch("0");
    rpm_sysvinit.set_version("2.88");
    rpm_sysvinit.set_release("14.dsf.fc20");
    rpm_sysvinit.set_arch("x86_64");
    rpm_sysvinit.set_repoid("f20");
    rpm_sysvinit.set_action(TransactionItemAction::OBSOLETED);
    rpm_sysvinit.set_reason(TransactionItemReason::USER);

    // TODO(dmach):
    // ti_rpm_sysvinit->addReplacedBy(ti_rpm_systemd);

    auto & comps_group_core = trans.new_comps_group();
    comps_group_core.set_group_id("core");
    comps_group_core.set_name("Core");
    comps_group_core.set_translated_name("Úplný základ");
    comps_group_core.set_repoid("");
    comps_group_core.set_action(TransactionItemAction::INSTALL);
    comps_group_core.set_reason(TransactionItemReason::USER);

    auto & core_bash = comps_group_core.new_package();
    core_bash.set_name("bash");
    core_bash.set_installed(true);
    core_bash.set_package_type(CompsPackageType::MANDATORY);

    auto & comps_environment_minimal = trans.new_comps_environment();
    comps_environment_minimal.set_environment_id("minimal");
    comps_environment_minimal.set_name("Minimal");
    comps_environment_minimal.set_translated_name("mmm");
    comps_environment_minimal.set_repoid("");
    comps_environment_minimal.set_action(TransactionItemAction::INSTALL);
    comps_environment_minimal.set_reason(TransactionItemReason::USER);

    auto & minimal_core = comps_environment_minimal.new_group();
    minimal_core.set_group_id("core");
    minimal_core.set_installed(true);
    minimal_core.set_group_type(CompsPackageType::MANDATORY);

    // STEP 4: save transaction and all associated items
    trans.begin();

    // STEP 5: record transaction output
    trans.add_console_output_line(1, "line1");
    trans.add_console_output_line(2, "line2");

    // STEP 6: run RPM transaction; callback: mark completed items
    for (auto & i : trans.get_comps_environments()) {
        i->set_state(TransactionItemState::DONE);
//        i->save();
    }
    for (auto & i : trans.get_comps_groups()) {
        i->set_state(TransactionItemState::DONE);
//        i->save();
    }
    for (auto & i : trans.get_packages()) {
        i->set_state(TransactionItemState::DONE);
//        i->save();
    }

    // STEP 6
    // mark completed transaction
    trans.finish(TransactionState::DONE);
    CPPUNIT_ASSERT_EQUAL(TransactionState::DONE, trans.get_state());

    /*
    // VERIFY
    // verify that data is available via public API
    auto trans2 = Transaction(*conn, trans.get_id());
    CPPUNIT_ASSERT_EQUAL(TransactionState::DONE, trans2.get_state());

    CPPUNIT_ASSERT(trans2.getItems().size() == 5);

    for (auto i : trans2.getItems()) {
        if (i->get_id() == 1) {
            CPPUNIT_ASSERT(i->get_action() == TransactionItemAction::INSTALL);
            CPPUNIT_ASSERT(i->get_reason() == TransactionItemReason::GROUP);
            CPPUNIT_ASSERT(i->get_repoid() == "base");
        } else if (i->get_id() == 2) {
            CPPUNIT_ASSERT(i->get_action() == TransactionItemAction::OBSOLETE);
            CPPUNIT_ASSERT(i->get_reason() == TransactionItemReason::USER);
            CPPUNIT_ASSERT(i->get_repoid() == "base");
        } else if (i->get_id() == 3) {
            CPPUNIT_ASSERT(i->get_action() == TransactionItemAction::OBSOLETED);
            CPPUNIT_ASSERT(i->get_reason() == TransactionItemReason::USER);
            CPPUNIT_ASSERT(i->get_repoid() == "f20");
        }

        // CPPUNIT_ASSERT(i->getItem()->getItemType() == "rpm");
        CPPUNIT_ASSERT_EQUAL(TransactionItemState::DONE, i->get_state());
        // std::cout << "TransactionItem: " << i->getItem()->toStr() << std::endl;
        if (i->getItem()->getItemType() == TransactionItemType::GROUP) {
            auto grp = std::dynamic_pointer_cast< CompsGroup >(i->getItem());
            CPPUNIT_ASSERT(grp->get_packages().size() == 1);
        }
        if (i->getItem()->getItemType() == TransactionItemType::ENVIRONMENT) {
            auto env = std::dynamic_pointer_cast< CompsEnvironment >(i->getItem());
            CPPUNIT_ASSERT(env->get_groups().size() == 1);
        }
    }

    auto console_output = trans2.get_console_output();
    CPPUNIT_ASSERT_EQUAL(2ul, console_output.size());
    */
}
