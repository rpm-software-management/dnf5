// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#include "test_goal.hpp"

#include "../shared/utils.hpp"

#include <libdnf5/base/goal.hpp>
#include <libdnf5/base/transaction_package.hpp>
#include <libdnf5/rpm/package_query.hpp>


CPPUNIT_TEST_SUITE_REGISTRATION(BaseGoalTest);

using namespace libdnf5::transaction;

void BaseGoalTest::setUp() {
    BaseTestCase::setUp();
}

void BaseGoalTest::test_install() {
    add_repo_repomd("repomd-repo1");

    libdnf5::Goal goal(base);
    goal.add_rpm_install("pkg");
    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {libdnf5::base::TransactionPackage(
        get_pkg("pkg-0:1.2-3.x86_64"),
        TransactionItemAction::INSTALL,
        TransactionItemReason::USER,
        TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());
}

void BaseGoalTest::test_install_not_available() {
    add_repo_repomd("repomd-repo1");

    libdnf5::Goal goal(base);
    base.get_config().get_skip_unavailable_option().set(true);
    base.get_config().get_best_option().set(true);
    base.get_config().get_clean_requirements_on_remove_option().set(true);
    goal.add_rpm_install("not_available");
    auto transaction = goal.resolve();

    CPPUNIT_ASSERT(transaction.get_transaction_packages().empty());

    auto & log = transaction.get_resolve_logs();
    CPPUNIT_ASSERT_EQUAL((size_t)1, log.size());
    auto & fist_event = *log.begin();
    CPPUNIT_ASSERT_EQUAL(libdnf5::GoalAction::INSTALL, fist_event.get_action());
    CPPUNIT_ASSERT_EQUAL(libdnf5::GoalProblem::NOT_FOUND, fist_event.get_problem());
    CPPUNIT_ASSERT_EQUAL(std::string("not_available"), *fist_event.get_spec());
    CPPUNIT_ASSERT_EQUAL(
        libdnf5::GoalUsedSetting::USED_FALSE, fist_event.get_job_settings()->get_used_clean_requirements_on_remove());
    CPPUNIT_ASSERT_EQUAL(libdnf5::GoalUsedSetting::USED_TRUE, fist_event.get_job_settings()->get_used_best());
    CPPUNIT_ASSERT_EQUAL(
        libdnf5::GoalUsedSetting::USED_TRUE, fist_event.get_job_settings()->get_used_skip_unavailable());
}

void BaseGoalTest::test_install_from_cmdline() {
    // Tests installing a cmdline package when there is a package with the same NEVRA available in a repo
    add_repo_rpm("rpm-repo1");
    auto cmdline_pkg = add_cmdline_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm");

    libdnf5::Goal goal(base);
    goal.add_rpm_install(cmdline_pkg);

    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {libdnf5::base::TransactionPackage(
        get_pkg("one-0:1-1.noarch", "@commandline"),
        TransactionItemAction::INSTALL,
        TransactionItemReason::USER,
        TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());
}

void BaseGoalTest::test_install_multilib_all() {
    add_repo_solv("solv-multilib");
    base.get_config().get_multilib_policy_option().set("all");

    libdnf5::Goal goal(base);
    goal.add_rpm_install("multilib");

    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {
        libdnf5::base::TransactionPackage(
            get_pkg("multilib-0:1.2-4.i686"),
            TransactionItemAction::INSTALL,
            TransactionItemReason::USER,
            TransactionItemState::STARTED),
        libdnf5::base::TransactionPackage(
            get_pkg("multilib-0:1.2-4.x86_64"),
            TransactionItemAction::INSTALL,
            TransactionItemReason::USER,
            TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());
}

void BaseGoalTest::test_reinstall() {
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm", TransactionItemReason::DEPENDENCY);

    libdnf5::Goal goal(base);
    goal.add_rpm_reinstall("one");

    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:1-1.noarch"),
            TransactionItemAction::REINSTALL,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED),
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:1-1.noarch", true),
            TransactionItemAction::REPLACED,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());
}

void BaseGoalTest::test_reinstall_from_cmdline() {
    // Tests the reinstallation using a cmdline package when a package with the same NEVRA is in available repo
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm", TransactionItemReason::DEPENDENCY);
    auto cmdline_pkg = add_cmdline_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm");

    libdnf5::Goal goal(base);
    goal.add_rpm_reinstall(cmdline_pkg);

    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:1-1.noarch", "@commandline"),
            TransactionItemAction::REINSTALL,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED),
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:1-1.noarch", true),
            TransactionItemAction::REPLACED,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());
}

void BaseGoalTest::test_reinstall_user() {
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm", TransactionItemReason::USER);

    libdnf5::Goal goal(base);
    goal.add_rpm_reinstall("one");

    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:1-1.noarch"),
            TransactionItemAction::REINSTALL,
            TransactionItemReason::USER,
            TransactionItemState::STARTED),
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:1-1.noarch", true),
            TransactionItemAction::REPLACED,
            TransactionItemReason::USER,
            TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());
}

void BaseGoalTest::test_remove() {
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm", TransactionItemReason::DEPENDENCY);

    libdnf5::Goal goal(base);
    goal.add_rpm_remove("one");
    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {libdnf5::base::TransactionPackage(
        get_pkg("one-0:1-1.noarch", true),
        TransactionItemAction::REMOVE,
        TransactionItemReason::USER,
        TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());
}

void BaseGoalTest::test_remove_not_installed() {
    base.get_config().get_clean_requirements_on_remove_option().set(true);

    libdnf5::Goal goal(base);
    goal.add_rpm_remove("not_installed");
    auto transaction = goal.resolve();

    CPPUNIT_ASSERT(transaction.get_transaction_packages().empty());

    auto & log = transaction.get_resolve_logs();
    CPPUNIT_ASSERT_EQUAL((size_t)1, log.size());
    auto & first_event = *log.begin();
    CPPUNIT_ASSERT_EQUAL(libdnf5::GoalAction::REMOVE, first_event.get_action());
    CPPUNIT_ASSERT_EQUAL(libdnf5::GoalProblem::NOT_FOUND, first_event.get_problem());
    CPPUNIT_ASSERT_EQUAL(std::string("not_installed"), *first_event.get_spec());
    CPPUNIT_ASSERT_EQUAL(
        libdnf5::GoalUsedSetting::USED_TRUE, first_event.get_job_settings()->get_used_clean_requirements_on_remove());
    CPPUNIT_ASSERT_EQUAL(libdnf5::GoalUsedSetting::UNUSED, first_event.get_job_settings()->get_used_best());
    CPPUNIT_ASSERT_EQUAL(libdnf5::GoalUsedSetting::UNUSED, first_event.get_job_settings()->get_used_skip_unavailable());
}

void BaseGoalTest::test_install_installed_pkg() {
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm", TransactionItemReason::DEPENDENCY);

    libdnf5::rpm::PackageQuery query(base);
    query.filter_available();
    query.filter_nevra("one-0:1-1.noarch");

    std::vector<libdnf5::rpm::Package> expected = {get_pkg("one-0:1-1.noarch")};
    CPPUNIT_ASSERT_EQUAL(expected, to_vector(query));

    libdnf5::Goal goal(base);
    goal.add_rpm_install(query);

    auto transaction = goal.resolve();

    CPPUNIT_ASSERT(transaction.get_transaction_packages().empty());
}

void BaseGoalTest::test_upgrade() {
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm", TransactionItemReason::DEPENDENCY);

    libdnf5::rpm::PackageQuery query(base);

    libdnf5::Goal goal(base);
    goal.add_rpm_upgrade("one");

    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:2-1.noarch"),
            TransactionItemAction::UPGRADE,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED),
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:1-1.noarch", true),
            TransactionItemAction::REPLACED,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());
}

void BaseGoalTest::test_upgrade_from_cmdline() {
    // Tests the upgrade using a cmdline package when a package with the same NEVRA is in available repo
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm", TransactionItemReason::DEPENDENCY);
    auto cmdline_pkg = add_cmdline_pkg("repos-rpm/rpm-repo1/one-2-1.noarch.rpm");

    libdnf5::rpm::PackageQuery query(base);

    libdnf5::Goal goal(base);
    goal.add_rpm_upgrade(cmdline_pkg);

    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:2-1.noarch", "@commandline"),
            TransactionItemAction::UPGRADE,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED),
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:1-1.noarch", true),
            TransactionItemAction::REPLACED,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());
}

void BaseGoalTest::test_upgrade_not_downgrade_from_cmdline() {
    // Tests the upgrade using a command line package when a newer version of package is installed
    // (downgrade must not be performed).
    // The package with the same NEVRA is also in available repo
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-2-1.noarch.rpm", TransactionItemReason::DEPENDENCY);
    auto cmdline_pkg = add_cmdline_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm");

    libdnf5::rpm::PackageQuery query(base);

    libdnf5::Goal goal(base);
    goal.add_rpm_upgrade(cmdline_pkg);

    auto transaction = goal.resolve();

    CPPUNIT_ASSERT(transaction.get_transaction_packages().empty());

    auto & log = transaction.get_resolve_logs();
    CPPUNIT_ASSERT_EQUAL((size_t)1, log.size());
    auto & first_event = *log.begin();
    CPPUNIT_ASSERT_EQUAL(libdnf5::GoalAction::UPGRADE, first_event.get_action());
    CPPUNIT_ASSERT_EQUAL(libdnf5::GoalProblem::ALREADY_INSTALLED, first_event.get_problem());
    CPPUNIT_ASSERT_EQUAL(std::string("one.noarch"), *first_event.get_additional_data().begin());
    CPPUNIT_ASSERT_EQUAL(
        libdnf5::GoalUsedSetting::USED_FALSE, first_event.get_job_settings()->get_used_clean_requirements_on_remove());
    CPPUNIT_ASSERT_EQUAL(libdnf5::GoalUsedSetting::USED_TRUE, first_event.get_job_settings()->get_used_best());
    CPPUNIT_ASSERT_EQUAL(libdnf5::GoalUsedSetting::UNUSED, first_event.get_job_settings()->get_used_skip_unavailable());
}

void BaseGoalTest::test_upgrade_not_available() {
    base.get_config().get_best_option().set(true);
    base.get_config().get_clean_requirements_on_remove_option().set(true);

    libdnf5::rpm::PackageQuery query(base);

    libdnf5::Goal goal(base);
    goal.add_rpm_upgrade("not_available");

    auto transaction = goal.resolve();

    CPPUNIT_ASSERT(transaction.get_transaction_packages().empty());

    auto & log = transaction.get_resolve_logs();
    CPPUNIT_ASSERT_EQUAL((size_t)1, log.size());
    auto & first_event = *log.begin();
    CPPUNIT_ASSERT_EQUAL(libdnf5::GoalAction::UPGRADE, first_event.get_action());
    CPPUNIT_ASSERT_EQUAL(libdnf5::GoalProblem::NOT_FOUND, first_event.get_problem());
    CPPUNIT_ASSERT_EQUAL(std::string("not_available"), *first_event.get_spec());
    CPPUNIT_ASSERT_EQUAL(
        libdnf5::GoalUsedSetting::USED_FALSE, first_event.get_job_settings()->get_used_clean_requirements_on_remove());
    CPPUNIT_ASSERT_EQUAL(libdnf5::GoalUsedSetting::USED_TRUE, first_event.get_job_settings()->get_used_best());
    CPPUNIT_ASSERT_EQUAL(
        libdnf5::GoalUsedSetting::USED_FALSE, first_event.get_job_settings()->get_used_skip_unavailable());
}

void BaseGoalTest::test_upgrade_all() {
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm", TransactionItemReason::DEPENDENCY);

    libdnf5::rpm::PackageQuery query(base);

    libdnf5::Goal goal(base);
    goal.add_rpm_upgrade();

    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:2-1.noarch"),
            TransactionItemAction::UPGRADE,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED),
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:1-1.noarch", true),
            TransactionItemAction::REPLACED,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());
}

void BaseGoalTest::test_upgrade_user() {
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm", TransactionItemReason::USER);

    libdnf5::rpm::PackageQuery query(base);

    libdnf5::Goal goal(base);
    goal.add_rpm_upgrade("one");

    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:2-1.noarch"),
            TransactionItemAction::UPGRADE,
            TransactionItemReason::USER,
            TransactionItemState::STARTED),
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:1-1.noarch", true),
            TransactionItemAction::REPLACED,
            TransactionItemReason::USER,
            TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());
}

void BaseGoalTest::test_downgrade() {
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-2-1.noarch.rpm", TransactionItemReason::DEPENDENCY);

    libdnf5::rpm::PackageQuery query(base);

    libdnf5::Goal goal(base);
    goal.add_rpm_downgrade("one");

    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:1-1.noarch"),
            TransactionItemAction::DOWNGRADE,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED),
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:2-1.noarch", true),
            TransactionItemAction::REPLACED,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());
}

void BaseGoalTest::test_downgrade_from_cmdline() {
    // Tests the downgrade using a cmdline package when a package with the same NEVRA is in available repo
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-2-1.noarch.rpm", TransactionItemReason::DEPENDENCY);
    auto cmdline_pkg = add_cmdline_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm");

    libdnf5::rpm::PackageQuery query(base);

    libdnf5::Goal goal(base);
    goal.add_rpm_downgrade(cmdline_pkg);

    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:1-1.noarch", "@commandline"),
            TransactionItemAction::DOWNGRADE,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED),
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:2-1.noarch", true),
            TransactionItemAction::REPLACED,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());
}

void BaseGoalTest::test_downgrade_user() {
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-2-1.noarch.rpm", TransactionItemReason::USER);

    libdnf5::rpm::PackageQuery query(base);

    libdnf5::Goal goal(base);
    goal.add_rpm_downgrade("one");

    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:1-1.noarch"),
            TransactionItemAction::DOWNGRADE,
            TransactionItemReason::USER,
            TransactionItemState::STARTED),
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:2-1.noarch", true),
            TransactionItemAction::REPLACED,
            TransactionItemReason::USER,
            TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());
}

void BaseGoalTest::test_distrosync() {
    add_repo_solv("solv-distrosync");
    add_system_pkg("repos-rpm/rpm-repo1/one-2-1.noarch.rpm", TransactionItemReason::DEPENDENCY);

    libdnf5::rpm::PackageQuery query(base);

    libdnf5::Goal goal(base);
    goal.add_rpm_distro_sync("one");

    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:1-1.noarch"),
            TransactionItemAction::DOWNGRADE,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED),
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:2-1.noarch", true),
            TransactionItemAction::REPLACED,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());
}

void BaseGoalTest::test_distrosync_all() {
    add_repo_solv("solv-distrosync");
    add_system_pkg("repos-rpm/rpm-repo1/one-2-1.noarch.rpm", TransactionItemReason::DEPENDENCY);

    libdnf5::rpm::PackageQuery query(base);

    libdnf5::Goal goal(base);
    goal.add_rpm_distro_sync();

    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:1-1.noarch"),
            TransactionItemAction::DOWNGRADE,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED),
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:2-1.noarch", true),
            TransactionItemAction::REPLACED,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());
}

void BaseGoalTest::test_install_or_reinstall() {
    add_repo_rpm("rpm-repo1");
    add_system_pkg("repos-rpm/rpm-repo1/one-1-1.noarch.rpm", TransactionItemReason::DEPENDENCY);

    libdnf5::Goal goal(base);
    libdnf5::rpm::PackageQuery query(base);
    query.filter_available();
    query.filter_nevra("one-0:1-1.noarch");
    CPPUNIT_ASSERT_EQUAL((size_t)1, query.size());
    goal.add_rpm_install_or_reinstall(query);
    auto transaction = goal.resolve();

    std::vector<libdnf5::base::TransactionPackage> expected = {
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:1-1.noarch"),
            TransactionItemAction::REINSTALL,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED),
        libdnf5::base::TransactionPackage(
            get_pkg("one-0:1-1.noarch", true),
            TransactionItemAction::REPLACED,
            TransactionItemReason::DEPENDENCY,
            TransactionItemState::STARTED)};
    CPPUNIT_ASSERT_EQUAL(expected, transaction.get_transaction_packages());
}
