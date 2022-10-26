/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "test_workflow.hpp"

#include "private_accessor.hpp"

#include "libdnf/common/sack/query_cmp.hpp"
#include "libdnf/comps/group/package.hpp"
#include "libdnf/transaction/transaction.hpp"

#include <string>


using namespace libdnf::transaction;


CPPUNIT_TEST_SUITE_REGISTRATION(TransactionWorkflowTest);

namespace {

// Allows accessing private methods
create_private_getter_template;
create_getter(new_package, &libdnf::transaction::Transaction::new_package);
create_getter(new_comps_group, &libdnf::transaction::Transaction::new_comps_group);
create_getter(new_comps_environment, &libdnf::transaction::Transaction::new_comps_environment);
create_getter(set_releasever, &libdnf::transaction::Transaction::set_releasever);
create_getter(start, &libdnf::transaction::Transaction::start);
create_getter(finish, &libdnf::transaction::Transaction::finish);

create_getter(new_transaction, &libdnf::transaction::TransactionHistory::new_transaction);

create_getter(set_name, &libdnf::transaction::Package::set_name);
create_getter(set_epoch, &libdnf::transaction::Package::set_epoch);
create_getter(set_version, &libdnf::transaction::Package::set_version);
create_getter(set_release, &libdnf::transaction::Package::set_release);
create_getter(set_arch, &libdnf::transaction::Package::set_arch);
create_getter(set_repoid, &libdnf::transaction::Package::set_repoid);
create_getter(set_action, &libdnf::transaction::Package::set_action);
create_getter(set_reason, &libdnf::transaction::Package::set_reason);
create_getter(set_state, &libdnf::transaction::Package::set_state);

}  //namespace

namespace CompsGroupPrivates {

create_private_getter_template;
create_getter(set_group_id, &libdnf::transaction::CompsGroup::set_group_id);
create_getter(set_name, &libdnf::transaction::CompsGroup::set_name);
create_getter(set_translated_name, &libdnf::transaction::CompsGroup::set_translated_name);
create_getter(set_repoid, &libdnf::transaction::CompsGroup::set_repoid);
create_getter(set_action, &libdnf::transaction::CompsGroup::set_action);
create_getter(set_reason, &libdnf::transaction::CompsGroup::set_reason);
create_getter(new_package, &libdnf::transaction::CompsGroup::new_package);

}  // namespace CompsGroupPrivates

namespace CompsGroupPackagePrivates {

create_private_getter_template;
create_getter(set_name, &libdnf::transaction::CompsGroupPackage::set_name);
create_getter(set_installed, &libdnf::transaction::CompsGroupPackage::set_installed);
create_getter(set_package_type, &libdnf::transaction::CompsGroupPackage::set_package_type);

}  // namespace CompsGroupPackagePrivates

namespace CompsEnvironmentPrivates {

create_private_getter_template;
create_getter(set_environment_id, &libdnf::transaction::CompsEnvironment::set_environment_id);
create_getter(set_name, &libdnf::transaction::CompsEnvironment::set_name);
create_getter(set_translated_name, &libdnf::transaction::CompsEnvironment::set_translated_name);
create_getter(set_repoid, &libdnf::transaction::CompsEnvironment::set_repoid);
create_getter(set_action, &libdnf::transaction::CompsEnvironment::set_action);
create_getter(set_reason, &libdnf::transaction::CompsEnvironment::set_reason);
create_getter(new_group, &libdnf::transaction::CompsEnvironment::new_group);

}  // namespace CompsEnvironmentPrivates
   //
namespace CompsEnvironmentGroupPrivates {

create_private_getter_template;
create_getter(set_group_id, &libdnf::transaction::CompsEnvironmentGroup::set_group_id);
create_getter(set_installed, &libdnf::transaction::CompsEnvironmentGroup::set_installed);
create_getter(set_group_type, &libdnf::transaction::CompsEnvironmentGroup::set_group_type);

}  // namespace CompsEnvironmentGroupPrivates

void TransactionWorkflowTest::test_default_workflow() {
    auto base = new_base();

    // create an empty Transaction object
    auto trans = (*(base->get_transaction_history()).*get(new_transaction{}))();
    CPPUNIT_ASSERT_EQUAL(TransactionState::STARTED, trans.get_state());

    // set vars
    (trans.*get(set_releasever{}))("26");

    // populate goal
    // resolve dependencies
    // prepare RPM transaction

    // add rpm packages to the transaction

    // bash-4.4.12-5.fc26.x86_64
    auto & rpm_bash = (trans.*get(new_package{}))();
    (rpm_bash.*get(set_name{}))("bash");
    (rpm_bash.*get(set_epoch{}))("0");
    (rpm_bash.*get(set_version{}))("4.4.12");
    (rpm_bash.*get(set_release{}))("5.fc26");
    (rpm_bash.*get(set_arch{}))("x86_64");
    (rpm_bash.*get(set_repoid{}))("base");
    (rpm_bash.*get(set_action{}))(TransactionItemAction::INSTALL);
    (rpm_bash.*get(set_reason{}))(TransactionItemReason::GROUP);

    // systemd-233-6.fc26
    auto & rpm_systemd = (trans.*get(new_package{}))();
    (rpm_systemd.*get(set_name{}))("systemd");
    (rpm_systemd.*get(set_epoch{}))("0");
    (rpm_systemd.*get(set_version{}))("233");
    (rpm_systemd.*get(set_release{}))("6.fc26");
    (rpm_systemd.*get(set_arch{}))("x86_64");
    (rpm_systemd.*get(set_repoid{}))("base");
    (rpm_systemd.*get(set_action{}))(TransactionItemAction::INSTALL);
    (rpm_systemd.*get(set_reason{}))(TransactionItemReason::USER);

    // sysvinit-2.88-14.dsf.fc20
    auto & rpm_sysvinit = (trans.*get(new_package{}))();
    (rpm_sysvinit.*get(set_name{}))("sysvinit");
    (rpm_sysvinit.*get(set_epoch{}))("0");
    (rpm_sysvinit.*get(set_version{}))("2.88");
    (rpm_sysvinit.*get(set_release{}))("14.dsf.fc20");
    (rpm_sysvinit.*get(set_arch{}))("x86_64");
    (rpm_sysvinit.*get(set_repoid{}))("f20");
    (rpm_sysvinit.*get(set_action{}))(TransactionItemAction::REPLACED);
    (rpm_sysvinit.*get(set_reason{}))(TransactionItemReason::USER);

    // TODO(dmach):
    // ti_rpm_sysvinit->addReplacedBy(ti_rpm_systemd);

    // add comps groups to the transaction

    auto & comps_group_core = (trans.*get(new_comps_group{}))();
    (comps_group_core.*get(CompsGroupPrivates::set_group_id{}))("core");
    (comps_group_core.*get(CompsGroupPrivates::set_name{}))("Core");
    (comps_group_core.*get(CompsGroupPrivates::set_translated_name{}))("Úplný základ");
    (comps_group_core.*get(CompsGroupPrivates::set_repoid{}))("");
    (comps_group_core.*get(CompsGroupPrivates::set_action{}))(TransactionItemAction::INSTALL);
    (comps_group_core.*get(CompsGroupPrivates::set_reason{}))(TransactionItemReason::USER);

    auto & core_bash = (comps_group_core.*get(CompsGroupPrivates::new_package{}))();
    (core_bash.*get(CompsGroupPackagePrivates::set_name{}))("bash");
    // When the Goal is resolved, we know if the package is part of the transaction or if it was installed already.
    // Both cases are equal from comps perspective and the group package must be marked as installed.
    // Please note that set_installed() has a completely different meaning than TransactionItemAction::INSTALL.
    (core_bash.*get(CompsGroupPackagePrivates::set_installed{}))(true);
    (core_bash.*get(CompsGroupPackagePrivates::set_package_type{}))(libdnf::comps::PackageType::MANDATORY);

    // add comps environments to the transaction

    auto & comps_environment_minimal = (trans.*get(new_comps_environment{}))();
    (comps_environment_minimal.*get(CompsEnvironmentPrivates::set_environment_id{}))("minimal");
    (comps_environment_minimal.*get(CompsEnvironmentPrivates::set_name{}))("Minimal");
    (comps_environment_minimal.*get(CompsEnvironmentPrivates::set_translated_name{}))("mmm");
    (comps_environment_minimal.*get(CompsEnvironmentPrivates::set_repoid{}))("");
    (comps_environment_minimal.*get(CompsEnvironmentPrivates::set_action{}))(TransactionItemAction::INSTALL);
    (comps_environment_minimal.*get(CompsEnvironmentPrivates::set_reason{}))(TransactionItemReason::USER);

    auto & minimal_core = (comps_environment_minimal.*get(CompsEnvironmentPrivates::new_group{}))();
    (minimal_core.*get(CompsEnvironmentGroupPrivates::set_group_id{}))("core");
    (minimal_core.*get(CompsEnvironmentGroupPrivates::set_installed{}))(true);
    (minimal_core.*get(CompsEnvironmentGroupPrivates::set_group_type{}))(libdnf::comps::PackageType::MANDATORY);

    // save transaction and all associated transaction items
    (trans.*get(start{}))();

    for (auto & env : trans.get_comps_environments()) {
        env.set_state(TransactionItemState::OK);
    }

    for (auto & grp : trans.get_comps_groups()) {
        grp.set_state(TransactionItemState::OK);
    }

    for (auto & pkg : trans.get_packages()) {
        pkg.set_state(TransactionItemState::OK);
    }

    // finish transaction
    (trans.*get(finish{}))(TransactionState::OK);
    CPPUNIT_ASSERT_EQUAL(TransactionState::OK, trans.get_state());
}
