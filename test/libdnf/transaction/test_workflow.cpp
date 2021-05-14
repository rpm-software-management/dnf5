/*
Copyright (C) 2017-2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "test_workflow.hpp"

#include "libdnf/common/sack/query_cmp.hpp"
#include "libdnf/transaction/transaction.hpp"

#include <string>


using namespace libdnf::transaction;


CPPUNIT_TEST_SUITE_REGISTRATION(TransactionWorkflowTest);


void TransactionWorkflowTest::test_default_workflow() {
    auto base = new_base();

    // create an empty Transaction object
    auto trans = base->get_transaction_sack()->new_transaction();
    CPPUNIT_ASSERT_EQUAL(TransactionState::UNKNOWN, trans->get_state());

    // set packages used to perform the transaction
    trans->add_runtime_package("dnf-3.5.1-1.fc29.noarch");

    // set vars
    trans->set_releasever("26");

    // populate goal
    // resolve dependencies
    // prepare RPM transaction

    // add rpm packages to the transaction

    // bash-4.4.12-5.fc26.x86_64
    auto & rpm_bash = trans->new_package();
    rpm_bash.set_name("bash");
    rpm_bash.set_epoch("0");
    rpm_bash.set_version("4.4.12");
    rpm_bash.set_release("5.fc26");
    rpm_bash.set_arch("x86_64");
    rpm_bash.set_repoid("base");
    rpm_bash.set_action(TransactionItemAction::INSTALL);
    rpm_bash.set_reason(TransactionItemReason::GROUP);

    // systemd-233-6.fc26
    auto & rpm_systemd = trans->new_package();
    rpm_systemd.set_name("systemd");
    rpm_systemd.set_epoch("0");
    rpm_systemd.set_version("233");
    rpm_systemd.set_release("6.fc26");
    rpm_systemd.set_arch("x86_64");
    rpm_systemd.set_repoid("base");
    rpm_systemd.set_action(TransactionItemAction::OBSOLETE);
    rpm_systemd.set_reason(TransactionItemReason::USER);

    // sysvinit-2.88-14.dsf.fc20
    auto & rpm_sysvinit = trans->new_package();
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

    // add comps groups to the transaction

    auto & comps_group_core = trans->new_comps_group();
    comps_group_core.set_group_id("core");
    comps_group_core.set_name("Core");
    comps_group_core.set_translated_name("Úplný základ");
    comps_group_core.set_repoid("");
    comps_group_core.set_action(TransactionItemAction::INSTALL);
    comps_group_core.set_reason(TransactionItemReason::USER);

    auto & core_bash = comps_group_core.new_package();
    core_bash.set_name("bash");
    // When the Goal is resolved, we know if the package is part of the transaction or if it was installed already.
    // Both cases are equal from comps perspective and the group package must be marked as installed.
    // Please note that set_installed() has a completely different meaning than TransactionItemAction::INSTALL.
    core_bash.set_installed(true);
    core_bash.set_package_type(CompsPackageType::MANDATORY);

    // add comps environments to the transaction

    auto & comps_environment_minimal = trans->new_comps_environment();
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

    // save transaction and all associated transaction items
    trans->start();

    // record transaction output coming from rpm transaction callbacks
    trans->add_console_output_line(1, "line1");
    trans->add_console_output_line(2, "line2");

    // run a rpm transaction
    // mark completed transaction items

    for (auto & it : trans->get_comps_environments()) {
        it->set_state(TransactionItemState::DONE);
        // it->save();
    }

    for (auto & it : trans->get_comps_groups()) {
        it->set_state(TransactionItemState::DONE);
        // it->save();
    }

    for (auto & it : trans->get_packages()) {
        it->set_state(TransactionItemState::DONE);
        // it->save();
    }

    // finish transaction
    trans->finish(TransactionState::DONE);
    CPPUNIT_ASSERT_EQUAL(TransactionState::DONE, trans->get_state());
}
