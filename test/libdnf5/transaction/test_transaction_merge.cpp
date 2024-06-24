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


#include "test_transaction_merge.hpp"

#include "../shared/utils.hpp"

#include <libdnf5/transaction/transaction.hpp>
#include <transaction/transaction_merge.hpp>

using namespace libdnf5::transaction;

CPPUNIT_TEST_SUITE_REGISTRATION(TransactionMergeTest);

static void add_transaction_item_package(
    TransactionReplay & trans,
    TransactionItemAction action,
    const std::string & nevra,
    const TransactionItemReason reason = TransactionItemReason::USER) {
    libdnf5::transaction::PackageReplay pkg_replay;
    pkg_replay.nevra = nevra;
    pkg_replay.action = action;
    pkg_replay.reason = reason;
    trans.packages.push_back(pkg_replay);
}

static void add_transaction_item_group(
    TransactionReplay & trans, TransactionItemAction action, const std::string & id) {
    libdnf5::transaction::GroupReplay group_replay;
    group_replay.action = action;
    group_replay.reason = TransactionItemReason::USER;
    group_replay.group_id = id;
    trans.groups.push_back(group_replay);
}

static void add_transaction_item_environment(
    TransactionReplay & trans, TransactionItemAction action, const std::string & id) {
    libdnf5::transaction::EnvironmentReplay env_replay;
    env_replay.action = action;
    env_replay.environment_id = id;
    trans.environments.push_back(env_replay);
}

static bool nevra_compare(PackageReplay pkg1, PackageReplay pkg2) {
    return (pkg1.nevra > pkg2.nevra);
};

void TransactionMergeTest::empty_transaction() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans;

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans}, na_to_installed_nevras);

    CPPUNIT_ASSERT_EQUAL((size_t)0, replay.packages.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, replay.groups.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, replay.environments.size());
    std::vector<std::string> expected_problems = {};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::only_one_transaction() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans;
    //bash-5.2.26-3.fc40.x86_64
    add_transaction_item_package(trans, TransactionItemAction::INSTALL, "bash-4-1.x86_64");
    add_transaction_item_package(trans, TransactionItemAction::INSTALL, "systemd-223-1.x86_64");
    add_transaction_item_package(trans, TransactionItemAction::INSTALL, "sysvinit-2-1.x86_64");
    add_transaction_item_group(trans, TransactionItemAction::INSTALL, "vlc");
    add_transaction_item_environment(trans, TransactionItemAction::INSTALL, "basic-desktop-environment");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans}, na_to_installed_nevras);

    std::sort(replay.packages.begin(), replay.packages.end(), nevra_compare);
    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "sysvinit-2-1.x86_64", "", ""},
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "systemd-223-1.x86_64", "", ""},
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<GroupReplay> expected_groups = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "vlc", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected_groups, replay.groups);
    std::vector<EnvironmentReplay> expected_envs = {
        {TransactionItemAction::INSTALL, "basic-desktop-environment", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected_envs, replay.environments);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::two_disjoint() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "bash-4-1.x86_64");
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "systemd-223-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::INSTALL, "sysvinit-2-1.x86_64");
    add_transaction_item_group(trans2, TransactionItemAction::INSTALL, "vlc");
    add_transaction_item_environment(trans2, TransactionItemAction::INSTALL, "basic-desktop-environment");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::sort(replay.packages.begin(), replay.packages.end(), nevra_compare);
    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "sysvinit-2-1.x86_64", "", ""},
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "systemd-223-1.x86_64", "", ""},
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<GroupReplay> expected_groups = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "vlc", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected_groups, replay.groups);
    std::vector<EnvironmentReplay> expected_envs = {
        {TransactionItemAction::INSTALL, "basic-desktop-environment", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected_envs, replay.environments);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::install_install() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::INSTALL, "bash-5-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-5-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        "Action 'Install' 'bash-5-1.x86_64' cannot be merged because it is already installed in version "
        "'bash-4-1.x86_64' -> keeping the action from older transaction with 'bash-5-1.x86_64'."};

    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);

    na_to_installed_nevras.clear();
    // Order matters when merging transacitons, we perfer the latest transaction
    auto [replay2, problems2] = libdnf5::transaction::merge_transactions({trans2, trans1}, na_to_installed_nevras);

    std::vector<PackageReplay> expected2 = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected2, replay2.packages);
    std::vector<std::string> expected_problems2 = {
        "Action 'Install' 'bash-4-1.x86_64' cannot be merged because it is already installed in version "
        "'bash-5-1.x86_64' -> keeping the action from older transaction with 'bash-4-1.x86_64'."};

    CPPUNIT_ASSERT_EQUAL(expected_problems2, problems2);
}

void TransactionMergeTest::install_install2() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::INSTALL, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        {"Action 'Install' 'bash-4-1.x86_64' cannot be merged after it was 'Install' in "
         "preceding transaction -> setting 'Install'."}};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::install_upgrade() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::UPGRADE, "bash-5-1.x86_64");
    add_transaction_item_package(trans2, TransactionItemAction::REPLACED, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-5-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::install_downgrade() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::DOWNGRADE, "bash-3-1.x86_64");
    add_transaction_item_package(trans2, TransactionItemAction::REPLACED, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-3-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::install_reinstall() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REINSTALL, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::install_remove() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REMOVE, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::install_replaced() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REPLACED, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::install_reasonchange() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(
        trans2, TransactionItemAction::REASON_CHANGE, "bash-4-1.x86_64", TransactionItemReason::DEPENDENCY);

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::DEPENDENCY, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::upgrade_install() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REPLACED, "bash-4-1.x86_64");
    add_transaction_item_package(trans1, TransactionItemAction::UPGRADE, "bash-5-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::INSTALL, "bash-5-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::sort(replay.packages.begin(), replay.packages.end(), nevra_compare);
    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-5-1.x86_64", "", ""},
        {TransactionItemAction::REPLACED, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        {"Action 'Install' 'bash-5-1.x86_64' cannot be merged after it was 'Install' in "
         "preceding transaction -> setting 'Install'."}};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}


void TransactionMergeTest::upgrade_upgrade() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REPLACED, "bash-4-1.x86_64");
    add_transaction_item_package(trans1, TransactionItemAction::UPGRADE, "bash-5-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REPLACED, "bash-5-1.x86_64");
    add_transaction_item_package(trans2, TransactionItemAction::UPGRADE, "bash-6-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::sort(replay.packages.begin(), replay.packages.end(), nevra_compare);
    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-6-1.x86_64", "", ""},
        {TransactionItemAction::REPLACED, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::upgrade_upgrade2() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REPLACED, "bash-4-1.x86_64");
    add_transaction_item_package(trans1, TransactionItemAction::UPGRADE, "bash-5-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REPLACED, "bash-4-1.x86_64");
    add_transaction_item_package(trans2, TransactionItemAction::UPGRADE, "bash-5-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::sort(replay.packages.begin(), replay.packages.end(), nevra_compare);
    std::vector<PackageReplay> expected = {
        {TransactionItemAction::UPGRADE, TransactionItemReason::USER, "", "bash-5-1.x86_64", "", ""},
        {TransactionItemAction::REPLACED, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        {"Action 'Replaced' 'bash-4-1.x86_64' cannot be merged after it was 'Replaced' in "
         "preceding transaction -> setting 'Replaced'."},
        {"Action 'Upgrade' 'bash-5-1.x86_64' cannot be merged after it was 'Install' in "
         "preceding transaction -> setting 'Upgrade'."}};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::upgrade_upgrade_installonly() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"kernel.x86_64", {"kernel-4-1.x86_64", "kernel-8-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REMOVE, "kernel-4-1.x86_64");
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "kernel-5-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REMOVE, "kernel-8-1.x86_64");
    add_transaction_item_package(trans2, TransactionItemAction::INSTALL, "kernel-9-1.x86_64");

    auto [replay, problems] =
        libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras, {"kernel"});

    std::sort(replay.packages.begin(), replay.packages.end(), nevra_compare);
    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "kernel-9-1.x86_64", "", ""},
        {TransactionItemAction::REMOVE, TransactionItemReason::USER, "", "kernel-8-1.x86_64", "", ""},
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "kernel-5-1.x86_64", "", ""},
        {TransactionItemAction::REMOVE, TransactionItemReason::USER, "", "kernel-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::upgrade_downgrade() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REPLACED, "bash-4-1.x86_64");
    add_transaction_item_package(trans1, TransactionItemAction::UPGRADE, "bash-5-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REPLACED, "bash-5-1.x86_64");
    add_transaction_item_package(trans2, TransactionItemAction::DOWNGRADE, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::sort(replay.packages.begin(), replay.packages.end(), nevra_compare);
    std::vector<PackageReplay> expected = {};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::upgrade_reinstall() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REPLACED, "bash-4-1.x86_64");
    add_transaction_item_package(trans1, TransactionItemAction::UPGRADE, "bash-5-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REINSTALL, "bash-5-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-5-1.x86_64", "", ""},
        {TransactionItemAction::REPLACED, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::upgrade_remove() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REPLACED, "bash-4-1.x86_64");
    add_transaction_item_package(trans1, TransactionItemAction::UPGRADE, "bash-5-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REMOVE, "bash-5-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::REPLACED, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::upgrade_replaced() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REPLACED, "bash-4-1.x86_64");
    add_transaction_item_package(trans1, TransactionItemAction::UPGRADE, "bash-5-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REPLACED, "bash-5-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::REPLACED, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::upgrade_reasonchange() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REPLACED, "bash-4-1.x86_64");
    add_transaction_item_package(trans1, TransactionItemAction::UPGRADE, "bash-5-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(
        trans2, TransactionItemAction::REASON_CHANGE, "bash-5-1.x86_64", TransactionItemReason::DEPENDENCY);

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::DEPENDENCY, "", "bash-5-1.x86_64", "", ""},
        {TransactionItemAction::REPLACED, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::upgrade_missing() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REPLACED, "bash-4-1.x86_64");
    add_transaction_item_package(trans1, TransactionItemAction::UPGRADE, "bash-5-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1}, na_to_installed_nevras);

    std::sort(replay.packages.begin(), replay.packages.end(), nevra_compare);
    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-5-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        "Action 'Replaced' 'bash-4-1.x86_64' cannot be merged because it is not present at that point -> skipping it.",
        "Action 'Upgrade' 'bash-5-1.x86_64' cannot be merged because it is not present at that point -> setting "
        "'INSTALL'."};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reinstall() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REINSTALL, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::REINSTALL, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reinstall_install() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REINSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::INSTALL, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::REINSTALL, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        {"Action 'Install' 'bash-4-1.x86_64' cannot be merged because it is already present at "
         "that point -> skipping it."}};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reinstall_install2() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REINSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::INSTALL, "bash-5-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-5-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        {"Action 'Install' 'bash-5-1.x86_64' cannot be merged because it is already installed in version "
         "'bash-4-1.x86_64' -> keeping the action from older transaction with 'bash-5-1.x86_64'."}};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reinstall_upgrade() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REINSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REPLACED, "bash-4-1.x86_64");
    add_transaction_item_package(trans2, TransactionItemAction::UPGRADE, "bash-5-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-5-1.x86_64", "", ""},
        {TransactionItemAction::REPLACED, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reinstall_downgrade() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REINSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REPLACED, "bash-4-1.x86_64");
    add_transaction_item_package(trans2, TransactionItemAction::INSTALL, "bash-3-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-3-1.x86_64", "", ""},
        {TransactionItemAction::REPLACED, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reinstall_reinstall() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REINSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REINSTALL, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::REINSTALL, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reinstall_remove() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REINSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REMOVE, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::REMOVE, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reinstall_replaced() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REINSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REPLACED, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::REPLACED, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reinstall_reasonchange() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REINSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(
        trans2, TransactionItemAction::REASON_CHANGE, "bash-4-1.x86_64", TransactionItemReason::DEPENDENCY);

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::REASON_CHANGE, TransactionItemReason::DEPENDENCY, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reinstall_missing() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REINSTALL, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        {"Action 'Reinstall' 'bash-4-1.x86_64' cannot be merged because it is not present at "
         "that point -> setting 'Install'."}};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reinstall_wrong_version() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-1-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REINSTALL, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        "Action 'Reinstall' 'bash-4-1.x86_64' cannot be merged because it is not present at that point (present "
        "versions are: bash-1-1.x86_64) -> skipping it."};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reasonchange() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(
        trans1, TransactionItemAction::REASON_CHANGE, "bash-4-1.x86_64", TransactionItemReason::DEPENDENCY);

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::REASON_CHANGE, TransactionItemReason::DEPENDENCY, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reasonchange_install() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(
        trans1, TransactionItemAction::REASON_CHANGE, "bash-4-1.x86_64", TransactionItemReason::DEPENDENCY);
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::INSTALL, "bash-5-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-5-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        "Action 'Install' 'bash-5-1.x86_64' cannot be merged because it is already installed in version "
        "'bash-4-1.x86_64' -> keeping the action from older transaction with 'bash-5-1.x86_64'."};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reasonchange_missing() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-1-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(
        trans1, TransactionItemAction::REASON_CHANGE, "bash-4-1.x86_64", TransactionItemReason::DEPENDENCY);

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        "Action 'Reason Change' 'bash-4-1.x86_64' cannot be merged because it is not present at that point (present "
        "versions are: bash-1-1.x86_64) -> skipping it."};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reasonchange_upgrade() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(
        trans1, TransactionItemAction::REASON_CHANGE, "bash-4-1.x86_64", TransactionItemReason::DEPENDENCY);
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::UPGRADE, "bash-5-1.x86_64");
    add_transaction_item_package(trans2, TransactionItemAction::REPLACED, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        // reason change is overwritten by the reason in UPGRADE
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-5-1.x86_64", "", ""},
        {TransactionItemAction::REPLACED, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reasonchange_downgrade() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(
        trans1, TransactionItemAction::REASON_CHANGE, "bash-4-1.x86_64", TransactionItemReason::DEPENDENCY);
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::DOWNGRADE, "bash-3-1.x86_64");
    add_transaction_item_package(trans2, TransactionItemAction::REPLACED, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        // reason change is overwritten by the reason in DOWNGRADE
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-3-1.x86_64", "", ""},
        {TransactionItemAction::REPLACED, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reasonchange_reinstall() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(
        trans1, TransactionItemAction::REASON_CHANGE, "bash-4-1.x86_64", TransactionItemReason::DEPENDENCY);
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REINSTALL, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        // reason change is overwritten by the reason in REINSTALL
        {TransactionItemAction::REINSTALL, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reasonchange_remove() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(
        trans1, TransactionItemAction::REASON_CHANGE, "bash-4-1.x86_64", TransactionItemReason::DEPENDENCY);
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REMOVE, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        // reason change is overwritten by the reason in remove
        {TransactionItemAction::REMOVE, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reasonchange_replaced() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(
        trans1, TransactionItemAction::REASON_CHANGE, "bash-4-1.x86_64", TransactionItemReason::DEPENDENCY);
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REPLACED, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        // reason change is overwritten by the reason in REPLACED
        {TransactionItemAction::REPLACED, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::reasonchange_reasonchange() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(
        trans1, TransactionItemAction::REASON_CHANGE, "bash-4-1.x86_64", TransactionItemReason::DEPENDENCY);
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(
        trans2, TransactionItemAction::REASON_CHANGE, "bash-4-1.x86_64", TransactionItemReason::WEAK_DEPENDENCY);

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::REASON_CHANGE, TransactionItemReason::WEAK_DEPENDENCY, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::remove_install() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REMOVE, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::INSTALL, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::remove_upgrade() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REMOVE, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REPLACED, "bash-4-1.x86_64");
    add_transaction_item_package(trans2, TransactionItemAction::UPGRADE, "bash-5-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-5-1.x86_64", "", ""},
        {TransactionItemAction::REPLACED, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        "Action 'Replaced' 'bash-4-1.x86_64' cannot be merged after it was 'Remove' in preceding transaction -> "
        "setting 'Replaced'.",
        "Action 'Upgrade' 'bash-5-1.x86_64' cannot be merged because it is not present at that point -> setting "
        "'INSTALL'."};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::remove_downgrade() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REMOVE, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REPLACED, "bash-4-1.x86_64");
    add_transaction_item_package(trans2, TransactionItemAction::DOWNGRADE, "bash-3-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-3-1.x86_64", "", ""},
        {TransactionItemAction::REPLACED, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        "Action 'Replaced' 'bash-4-1.x86_64' cannot be merged after it was 'Remove' in preceding transaction -> "
        "setting 'Replaced'.",
        "Action 'Downgrade' 'bash-3-1.x86_64' cannot be merged because it is not present at that point -> setting "
        "'INSTALL'."};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::remove_reinstall() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REMOVE, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REINSTALL, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        {"Action 'Reinstall' 'bash-4-1.x86_64' cannot be merged because it is not present at "
         "that point -> setting 'Install'."}};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::remove_remove() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REMOVE, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REMOVE, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::REMOVE, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        {"Action 'Remove' 'bash-4-1.x86_64' cannot be merged after it was 'Remove' in "
         "preceding transaction -> setting 'Remove'."}};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::remove_remove_installonly() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"kernel.x86_64", {"kernel-4-1.x86_64", "kernel-5-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REMOVE, "kernel-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REMOVE, "kernel-5-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::REMOVE, TransactionItemReason::USER, "", "kernel-5-1.x86_64", "", ""},
        {TransactionItemAction::REMOVE, TransactionItemReason::USER, "", "kernel-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::remove_replaced() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REMOVE, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REPLACED, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::REPLACED, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        {"Action 'Replaced' 'bash-4-1.x86_64' cannot be merged after it was 'Remove' in "
         "preceding transaction -> setting 'Replaced'."}};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::remove_reasonchange() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REMOVE, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(
        trans2, TransactionItemAction::REASON_CHANGE, "bash-4-1.x86_64", TransactionItemReason::DEPENDENCY);

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::REMOVE, TransactionItemReason::DEPENDENCY, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::remove_missing() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-1-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REMOVE, "bash-4-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        "Action 'Remove' 'bash-4-1.x86_64' cannot be merged because it is not present at that point (present versions "
        "are: bash-1-1.x86_64) -> skipping it."};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::install_upgrade_reinstall() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REPLACED, "bash-4-1.x86_64");
    add_transaction_item_package(trans2, TransactionItemAction::UPGRADE, "bash-5-1.x86_64");
    libdnf5::transaction::TransactionReplay trans3;
    add_transaction_item_package(trans3, TransactionItemAction::REINSTALL, "bash-5-1.x86_64");

    auto [replay, problems] =
        libdnf5::transaction::merge_transactions({trans1, trans2, trans3}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-5-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::remove_install_remove() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REMOVE, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::INSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans3;
    add_transaction_item_package(trans3, TransactionItemAction::REMOVE, "bash-4-1.x86_64");

    auto [replay, problems] =
        libdnf5::transaction::merge_transactions({trans1, trans2, trans3}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::REMOVE, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::install_reasonchange_upgrade() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(
        trans2, TransactionItemAction::REASON_CHANGE, "bash-4-1.x86_64", TransactionItemReason::DEPENDENCY);
    libdnf5::transaction::TransactionReplay trans3;
    add_transaction_item_package(trans3, TransactionItemAction::UPGRADE, "bash-5-1.x86_64");
    add_transaction_item_package(trans3, TransactionItemAction::REPLACED, "bash-4-1.x86_64");

    auto [replay, problems] =
        libdnf5::transaction::merge_transactions({trans1, trans2, trans3}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        // the reason change is overwritten by the UPGRADE reason
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-5-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::install_remove_install() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REMOVE, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans3;
    add_transaction_item_package(trans3, TransactionItemAction::INSTALL, "bash-4-1.x86_64");

    auto [replay, problems] =
        libdnf5::transaction::merge_transactions({trans1, trans2, trans3}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::install_remove_upgrade() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::INSTALL, "bash-5-1.x86_64");
    libdnf5::transaction::TransactionReplay trans3;
    add_transaction_item_package(trans3, TransactionItemAction::REMOVE, "bash-5-1.x86_64");

    auto [replay, problems] =
        libdnf5::transaction::merge_transactions({trans1, trans2, trans3}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        "Action 'Install' 'bash-5-1.x86_64' cannot be merged because it is already installed in version "
        "'bash-4-1.x86_64' -> keeping the action from older transaction with 'bash-5-1.x86_64'."};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::install_install_remove() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REMOVE, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans3;
    add_transaction_item_package(trans3, TransactionItemAction::REPLACED, "bash-4-1.x86_64");
    add_transaction_item_package(trans3, TransactionItemAction::UPGRADE, "bash-5-1.x86_64");

    auto [replay, problems] =
        libdnf5::transaction::merge_transactions({trans1, trans2, trans3}, na_to_installed_nevras);

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-5-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        "Action 'Replaced' 'bash-4-1.x86_64' cannot be merged because it is not present at that point -> skipping it.",
        "Action 'Upgrade' 'bash-5-1.x86_64' cannot be merged because it is not present at that point -> setting "
        "'INSTALL'."};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::remove_install_lower() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"bash.x86_64", {"bash-4-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REMOVE, "bash-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::INSTALL, "bash-3-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::sort(replay.packages.begin(), replay.packages.end(), nevra_compare);
    std::vector<PackageReplay> expected = {
        {TransactionItemAction::REMOVE, TransactionItemReason::USER, "", "bash-4-1.x86_64", "", ""},
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "bash-3-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::install_install_installonly() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "kernel-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::INSTALL, "kernel-5-1.x86_64");

    auto [replay, problems] =
        libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras, {"kernel"});

    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "kernel-5-1.x86_64", "", ""},
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "kernel-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::install_remove_install_installonly() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "kernel-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::INSTALL, "kernel-5-1.x86_64");
    libdnf5::transaction::TransactionReplay trans3;
    add_transaction_item_package(trans3, TransactionItemAction::REMOVE, "kernel-4-1.x86_64");
    add_transaction_item_package(trans3, TransactionItemAction::REMOVE, "kernel-5-1.x86_64");
    libdnf5::transaction::TransactionReplay trans4;
    add_transaction_item_package(trans4, TransactionItemAction::INSTALL, "kernel-4-1.x86_64");
    add_transaction_item_package(trans4, TransactionItemAction::INSTALL, "kernel-5-1.x86_64");

    auto [replay, problems] =
        libdnf5::transaction::merge_transactions({trans1, trans2, trans3, trans4}, na_to_installed_nevras, {"kernel"});

    std::sort(replay.packages.begin(), replay.packages.end(), nevra_compare);
    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "kernel-5-1.x86_64", "", ""},
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "kernel-4-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::remove_installonly_missing() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REMOVE, "kernel-4-1.x86_64");
    add_transaction_item_package(trans1, TransactionItemAction::REMOVE, "kernel-5-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1}, na_to_installed_nevras, {"kernel"});

    std::sort(replay.packages.begin(), replay.packages.end(), nevra_compare);
    std::vector<PackageReplay> expected = {};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    std::vector<std::string> expected_problems = {
        "Action 'Remove' 'kernel-4-1.x86_64' cannot be merged because it is not present at that point -> skipping it.",
        "Action 'Remove' 'kernel-5-1.x86_64' cannot be merged because it is not present at that point -> skipping it."};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::install_remove_installonly() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "kernel-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::INSTALL, "kernel-5-1.x86_64");
    libdnf5::transaction::TransactionReplay trans3;
    add_transaction_item_package(trans3, TransactionItemAction::REMOVE, "kernel-4-1.x86_64");
    add_transaction_item_package(trans3, TransactionItemAction::REMOVE, "kernel-5-1.x86_64");

    auto [replay, problems] =
        libdnf5::transaction::merge_transactions({trans1, trans2, trans3}, na_to_installed_nevras, {"kernel"});

    std::sort(replay.packages.begin(), replay.packages.end(), nevra_compare);
    std::vector<PackageReplay> expected = {};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::upgrade_installonly_forward() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"kernel.x86_64", {"kernel-1-1.x86_64", "kernel-2-1.x86_64", "kernel-3-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REMOVE, "kernel-1-1.x86_64");
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "kernel-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REMOVE, "kernel-2-1.x86_64");
    add_transaction_item_package(trans2, TransactionItemAction::INSTALL, "kernel-5-1.x86_64");
    libdnf5::transaction::TransactionReplay trans3;
    add_transaction_item_package(trans3, TransactionItemAction::REMOVE, "kernel-3-1.x86_64");
    add_transaction_item_package(trans3, TransactionItemAction::INSTALL, "kernel-6-1.x86_64");
    libdnf5::transaction::TransactionReplay trans4;
    add_transaction_item_package(trans4, TransactionItemAction::REMOVE, "kernel-4-1.x86_64");
    add_transaction_item_package(trans4, TransactionItemAction::INSTALL, "kernel-7-1.x86_64");
    libdnf5::transaction::TransactionReplay trans5;
    add_transaction_item_package(trans5, TransactionItemAction::REMOVE, "kernel-5-1.x86_64");
    add_transaction_item_package(trans5, TransactionItemAction::INSTALL, "kernel-8-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions(
        {trans1, trans2, trans3, trans4, trans5}, na_to_installed_nevras, {"kernel"});

    std::sort(replay.packages.begin(), replay.packages.end(), nevra_compare);
    std::vector<PackageReplay> expected = {
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "kernel-8-1.x86_64", "", ""},
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "kernel-7-1.x86_64", "", ""},
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "kernel-6-1.x86_64", "", ""},
        {TransactionItemAction::REMOVE, TransactionItemReason::USER, "", "kernel-3-1.x86_64", "", ""},
        {TransactionItemAction::REMOVE, TransactionItemReason::USER, "", "kernel-2-1.x86_64", "", ""},
        {TransactionItemAction::REMOVE, TransactionItemReason::USER, "", "kernel-1-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::upgrade_installonly_backward() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras{
        {"kernel.x86_64", {"kernel-7-1.x86_64", "kernel-8-1.x86_64", "kernel-9-1.x86_64"}}};

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_package(trans1, TransactionItemAction::REMOVE, "kernel-9-1.x86_64");
    add_transaction_item_package(trans1, TransactionItemAction::INSTALL, "kernel-6-1.x86_64");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_package(trans2, TransactionItemAction::REMOVE, "kernel-8-1.x86_64");
    add_transaction_item_package(trans2, TransactionItemAction::INSTALL, "kernel-5-1.x86_64");
    libdnf5::transaction::TransactionReplay trans3;
    add_transaction_item_package(trans3, TransactionItemAction::REMOVE, "kernel-7-1.x86_64");
    add_transaction_item_package(trans3, TransactionItemAction::INSTALL, "kernel-4-1.x86_64");
    libdnf5::transaction::TransactionReplay trans4;
    add_transaction_item_package(trans4, TransactionItemAction::REMOVE, "kernel-6-1.x86_64");
    add_transaction_item_package(trans4, TransactionItemAction::INSTALL, "kernel-3-1.x86_64");
    libdnf5::transaction::TransactionReplay trans5;
    add_transaction_item_package(trans5, TransactionItemAction::REMOVE, "kernel-5-1.x86_64");
    add_transaction_item_package(trans5, TransactionItemAction::INSTALL, "kernel-2-1.x86_64");

    auto [replay, problems] = libdnf5::transaction::merge_transactions(
        {trans1, trans2, trans3, trans4, trans5}, na_to_installed_nevras, {"kernel"});

    std::sort(replay.packages.begin(), replay.packages.end(), nevra_compare);
    std::vector<PackageReplay> expected = {
        {TransactionItemAction::REMOVE, TransactionItemReason::USER, "", "kernel-9-1.x86_64", "", ""},
        {TransactionItemAction::REMOVE, TransactionItemReason::USER, "", "kernel-8-1.x86_64", "", ""},
        {TransactionItemAction::REMOVE, TransactionItemReason::USER, "", "kernel-7-1.x86_64", "", ""},
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "kernel-4-1.x86_64", "", ""},
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "kernel-3-1.x86_64", "", ""},
        {TransactionItemAction::INSTALL, TransactionItemReason::USER, "", "kernel-2-1.x86_64", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.packages);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::group_install_remove() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_group(trans1, TransactionItemAction::INSTALL, "vlc");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_group(trans2, TransactionItemAction::REMOVE, "vlc");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<GroupReplay> expected = {};
    CPPUNIT_ASSERT_EQUAL(expected, replay.groups);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}


void TransactionMergeTest::group_install_install() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_group(trans1, TransactionItemAction::INSTALL, "vlc");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_group(trans2, TransactionItemAction::INSTALL, "vlc");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<GroupReplay> expected = {{TransactionItemAction::INSTALL, TransactionItemReason::USER, "vlc", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.groups);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::group_install_upgrade() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_group(trans1, TransactionItemAction::INSTALL, "vlc");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_group(trans2, TransactionItemAction::UPGRADE, "vlc");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<GroupReplay> expected = {{TransactionItemAction::INSTALL, TransactionItemReason::USER, "vlc", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.groups);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::group_remove_install() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_group(trans1, TransactionItemAction::REMOVE, "vlc");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_group(trans2, TransactionItemAction::INSTALL, "vlc");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<GroupReplay> expected = {};
    CPPUNIT_ASSERT_EQUAL(expected, replay.groups);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::group_remove_upgrade() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_group(trans1, TransactionItemAction::REMOVE, "vlc");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_group(trans2, TransactionItemAction::UPGRADE, "vlc");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<GroupReplay> expected = {{TransactionItemAction::INSTALL, TransactionItemReason::USER, "vlc", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.groups);
    std::vector<std::string> expected_problems = {
        {"Action 'Upgrade' 'vlc' cannot be merged because it is not present at that point -> "
         "setting 'Install'."}};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::group_remove_remove() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_group(trans1, TransactionItemAction::REMOVE, "vlc");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_group(trans2, TransactionItemAction::REMOVE, "vlc");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<GroupReplay> expected = {{TransactionItemAction::REMOVE, TransactionItemReason::USER, "vlc", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.groups);
    std::vector<std::string> expected_problems = {
        {"Action 'Remove' 'vlc' cannot be merged after it was 'Remove' in preceding "
         "transactions -> setting 'Remove'."}};
    CPPUNIT_ASSERT_EQUAL(expected_problems, problems);
}

void TransactionMergeTest::group_upgrade_upgrade() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_group(trans1, TransactionItemAction::UPGRADE, "vlc");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_group(trans2, TransactionItemAction::UPGRADE, "vlc");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<GroupReplay> expected = {{TransactionItemAction::UPGRADE, TransactionItemReason::USER, "vlc", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.groups);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::group_upgrade_remove() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_group(trans1, TransactionItemAction::UPGRADE, "vlc");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_group(trans2, TransactionItemAction::REMOVE, "vlc");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<GroupReplay> expected = {{TransactionItemAction::REMOVE, TransactionItemReason::USER, "vlc", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.groups);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::group_upgrade_install() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_group(trans1, TransactionItemAction::UPGRADE, "vlc");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_group(trans2, TransactionItemAction::INSTALL, "vlc");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<GroupReplay> expected = {{TransactionItemAction::INSTALL, TransactionItemReason::USER, "vlc", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.groups);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::env_install_remove() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_environment(trans1, TransactionItemAction::INSTALL, "vlc");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_environment(trans2, TransactionItemAction::REMOVE, "vlc");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<EnvironmentReplay> expected = {};
    CPPUNIT_ASSERT_EQUAL(expected, replay.environments);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}

void TransactionMergeTest::env_upgrade_remove() {
    std::unordered_map<std::string, std::vector<std::string>> na_to_installed_nevras;

    libdnf5::transaction::TransactionReplay trans1;
    add_transaction_item_environment(trans1, TransactionItemAction::UPGRADE, "vlc");
    libdnf5::transaction::TransactionReplay trans2;
    add_transaction_item_environment(trans2, TransactionItemAction::REMOVE, "vlc");

    auto [replay, problems] = libdnf5::transaction::merge_transactions({trans1, trans2}, na_to_installed_nevras);

    std::vector<EnvironmentReplay> expected = {{TransactionItemAction::REMOVE, "vlc", "", ""}};
    CPPUNIT_ASSERT_EQUAL(expected, replay.environments);
    CPPUNIT_ASSERT_EQUAL(std::vector<std::string>(), problems);
}
