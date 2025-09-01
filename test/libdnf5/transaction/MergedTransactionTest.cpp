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

// TODO(dmach): keep refactoring and deliver something that works with the new code base
// the whole file is disabled via the SKIP macro because it doesn't compile with the new code
#ifdef SKIP

#include <set>
#include <string>

//#include <libdnf5/hy-subject.h>
#include "MergedTransactionTest.hpp"

#include <libdnf5/rpm/nevra.hpp>
#include <libdnf5/transaction/MergedTransaction.hpp>
#include <libdnf5/transaction/Transformer.hpp>
#include <libdnf5/transaction/rpm_package.hpp>
#include <libdnf5/transaction/transaction.hpp>

using namespace libdnf5::transaction;

CPPUNIT_TEST_SUITE_REGISTRATION(MergedTransactionTest);

void MergedTransactionTest::setUp() {
    conn = new libdnf5::utils::SQLite3(":memory:");
    Transformer::createDatabase(*conn);
}

static TransactionPtr initTransFirst(libdnf5::utils::SQLite3 & conn) {
    // create the first transaction
    auto first = std::make_shared<Transaction>(conn);
    first->set_dt_begin(1);
    first->set_dt_end(2);
    first->set_rpmdb_version_begin("begin 1");
    first->set_rpmdb_version_end("end 1");
    first->set_user_id(1000);
    first->set_cmdline("dnf install foo");
    first->add_runtime_package("dnf-3.0.0-2.fc26.x86_64");
    return first;
}

static TransactionPtr initTransSecond(libdnf5::utils::SQLite3 & conn) {
    // create the second transaction
    auto second = std::make_shared<Transaction>(conn);
    second->set_dt_begin(3);
    second->set_dt_end(4);
    second->set_rpmdb_version_begin("begin 2");
    second->set_rpmdb_version_end("end 2");
    second->set_user_id(1001);
    second->set_cmdline("dnf install bar");
    second->add_runtime_package("rpm-4.14.0-2.fc26.x86_64");
    return second;
}

void MergedTransactionTest::tearDown() {
    delete conn;
}

void MergedTransactionTest::testMerge() {
    auto first = initTransFirst(*conn);
    first->start();
    first->add_console_output_line(1, "Foo");
    first->finish(TransactionState::DONE);

    auto second = initTransSecond(*conn);
    second->start();
    second->add_console_output_line(1, "Bar");
    second->finish(TransactionState::ERROR);

    MergedTransaction merged(*first);
    merged.merge(*second);

    CPPUNIT_ASSERT_EQUAL((int64_t)1, merged.listIds().at(0));
    CPPUNIT_ASSERT_EQUAL((int64_t)2, merged.listIds().at(1));

    CPPUNIT_ASSERT_EQUAL((uint32_t)1000, merged.listUserIds().at(0));
    CPPUNIT_ASSERT_EQUAL((uint32_t)1001, merged.listUserIds().at(1));

    CPPUNIT_ASSERT_EQUAL(std::string("dnf install foo"), merged.listCmdlines().at(0));
    CPPUNIT_ASSERT_EQUAL(std::string("dnf install bar"), merged.listCmdlines().at(1));

    CPPUNIT_ASSERT_EQUAL(TransactionState::DONE, merged.listStates().at(0));
    CPPUNIT_ASSERT_EQUAL(TransactionState::ERROR, merged.listStates().at(1));

    CPPUNIT_ASSERT_EQUAL((int64_t)1, merged.get_dt_begin());
    CPPUNIT_ASSERT_EQUAL((int64_t)4, merged.get_dt_end());

    CPPUNIT_ASSERT_EQUAL(std::string("begin 1"), merged.get_rpmdb_version_begin());
    CPPUNIT_ASSERT_EQUAL(std::string("end 2"), merged.get_rpmdb_version_end());

    auto output = merged.get_console_output();
    CPPUNIT_ASSERT_EQUAL(1, output.at(0).first);
    CPPUNIT_ASSERT_EQUAL(std::string("Foo"), output.at(0).second);

    CPPUNIT_ASSERT_EQUAL(1, output.at(1).first);
    CPPUNIT_ASSERT_EQUAL(std::string("Bar"), output.at(1).second);

    auto software = merged.get_runtime_packages();
    std::set<std::string> packages = {"dnf-3.0.0-2.fc26.x86_64", "rpm-4.14.0-2.fc26.x86_64"};

    CPPUNIT_ASSERT_EQUAL((size_t)2, packages.size());

    for (auto & nevra : software) {
        CPPUNIT_ASSERT_MESSAGE("Package: " + nevra, packages.find(nevra) != packages.end());
        packages.erase(nevra);
    }
}

static MergedTransactionPtr prepareMergedTransaction(
    libdnf5::utils::SQLite3 & conn,
    TransactionItemAction actionFirst,
    TransactionItemAction actionSecond,
    const std::string & versionFirst,
    const std::string & versionSecond,
    TransactionItemAction oldFirstAction = TransactionItemAction::INSTALL,
    const std::string & oldFirstVersion = {}

) {
    auto first = initTransFirst(conn);
    auto firstRPM = std::make_shared<Package>(*first);
    firstRPM->set_name("foo");
    firstRPM->set_epoch("0");
    firstRPM->set_version(versionFirst);
    firstRPM->set_release("2.fc26");
    firstRPM->set_arch("x86_64");
    firstRPM->save();

    auto second = initTransSecond(conn);
    auto secondRPM = std::make_shared<Package>(*second);
    secondRPM->set_name("foo");
    secondRPM->set_epoch("0");
    secondRPM->set_version(versionSecond);
    secondRPM->set_release("2.fc26");
    secondRPM->set_arch("x86_64");
    secondRPM->save();

    if (oldFirstAction != TransactionItemAction::INSTALL) {
        auto oldFirst = std::make_shared<Package>(*first);
        oldFirst->set_name("foo");
        oldFirst->set_epoch("0");
        oldFirst->set_version(oldFirstVersion);
        oldFirst->set_release("2.fc26");
        oldFirst->set_arch("x86_64");
        oldFirst->save();
        first->addItem(oldFirst, "base", oldFirstAction, TransactionItemReason::USER);
    }

    first->addItem(firstRPM, "base", actionFirst, TransactionItemReason::USER);
    first->start();
    first->finish(TransactionState::DONE);

    second->addItem(secondRPM, "base", actionSecond, TransactionItemReason::USER);
    second->start();
    second->finish(TransactionState::DONE);

    auto merged = std::make_shared<MergedTransaction>(*first);
    merged->merge(*second);
    return merged;
}

/// Erase -> Install = Reinstall
void MergedTransactionTest::testMergeEraseInstallReinstall() {
    auto merged = prepareMergedTransaction(
        *conn, TransactionItemAction::REMOVE, TransactionItemAction::INSTALL, "1.0.0", "1.0.0");

    auto items = merged->getItems();
    CPPUNIT_ASSERT_EQUAL(1, (int)items.size());
    auto item = items.at(0);
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::REINSTALL, item->get_action());
}

/// Erase -> Install = Downgrade
void MergedTransactionTest::testMergeEraseInstallDowngrade() {
    auto merged = prepareMergedTransaction(
        *conn, TransactionItemAction::REMOVE, TransactionItemAction::INSTALL, "0.11.0", "0.10.9");

    auto items = merged->getItems();

    CPPUNIT_ASSERT_EQUAL(1, (int)items.size());
    auto item = items.at(0);
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::DOWNGRADE, item->get_action());
}

/// Erase -> Install = Upgrade
void MergedTransactionTest::testMergeEraseInstallUpgrade() {
    auto merged = prepareMergedTransaction(
        *conn, TransactionItemAction::OBSOLETED, TransactionItemAction::INSTALL, "0.11.0", "0.12.0");

    auto items = merged->getItems();
    CPPUNIT_ASSERT_EQUAL(1, (int)items.size());
    auto item = items.at(0);
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::UPGRADE, item->get_action());
}

/// Reinstall/Reason change -> (new action) = (new action)
void MergedTransactionTest::testMergeReinstallAny() {
    auto merged = prepareMergedTransaction(
        *conn,
        TransactionItemAction::REINSTALL,
        TransactionItemAction::UPGRADE,
        "1.0.0",
        "1.0.1",
        TransactionItemAction::UPGRADED,
        "0.9.9");

    auto items = merged->getItems();
    CPPUNIT_ASSERT_EQUAL(2, (int)items.size());
    auto item = items.at(1);
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::UPGRADE, item->get_action());
    auto oldItem = items.at(0);
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::UPGRADED, oldItem->get_action());
}

/// Install -> Erase = (nothing)
void MergedTransactionTest::testMergeInstallErase() {
    auto merged = prepareMergedTransaction(
        *conn, TransactionItemAction::INSTALL, TransactionItemAction::REMOVE, "1.0.0", "1.0.0");

    auto items = merged->getItems();

    // nothing
    CPPUNIT_ASSERT_EQUAL(0, (int)items.size());
}

/// Install -> Upgrade/Downgrade = Install (with Upgrade version)

void MergedTransactionTest::testMergeInstallAlter() {
    auto merged = prepareMergedTransaction(
        *conn, TransactionItemAction::INSTALL, TransactionItemAction::UPGRADE, "1.0.0", "1.0.1");

    auto items = merged->getItems();
    CPPUNIT_ASSERT_EQUAL(1, (int)items.size());
    auto item = items.at(0);
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::INSTALL, item->get_action());
    auto rpm = std::dynamic_pointer_cast<Package>(item->getItem());
    CPPUNIT_ASSERT_EQUAL(std::string("1.0.1"), rpm->get_version());
}

/// Downgrade/Upgrade/Obsoleting -> Reinstall = (old action)
void MergedTransactionTest::testMergeAlterReinstall() {
    auto merged = prepareMergedTransaction(
        *conn,
        TransactionItemAction::UPGRADE,
        TransactionItemAction::REINSTALL,
        "1.0.0",
        "1.0.0",
        TransactionItemAction::UPGRADED,
        "0.9.9");

    auto items = merged->getItems();
    CPPUNIT_ASSERT_EQUAL(2, (int)items.size());
    auto item = items.at(1);
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::UPGRADE, item->get_action());
    auto oldItem = items.at(0);
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::UPGRADED, oldItem->get_action());
}

/// Downgrade/Upgrade/Obsoleting -> Erase/Obsoleted = Erase/Obsolete (with old package)
void MergedTransactionTest::testMergeAlterErase() {
    auto merged = prepareMergedTransaction(
        *conn,
        TransactionItemAction::UPGRADE,
        TransactionItemAction::REMOVE,
        "1.0.0",
        "1.0.0",
        TransactionItemAction::UPGRADED,
        "0.9.9");
    auto items = merged->getItems();
    CPPUNIT_ASSERT_EQUAL(1, (int)items.size());
    auto item = items.at(0);
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::REMOVE, item->get_action());
    auto rpm = std::dynamic_pointer_cast<Package>(item->getItem());
    CPPUNIT_ASSERT_EQUAL(std::string("0.9.9"), rpm->get_version());
}

/// Downgrade/Upgrade/Obsoleting -> Downgrade/Upgrade = Downgrade/Upgrade/Reinstall
void MergedTransactionTest::testMergeAlterAlter() {
    auto merged = prepareMergedTransaction(
        *conn,
        TransactionItemAction::DOWNGRADE,
        TransactionItemAction::UPGRADE,
        "1.0.0",
        "1.0.1",
        TransactionItemAction::DOWNGRADED,
        "1.0.1");
    auto items = merged->getItems();
    CPPUNIT_ASSERT_EQUAL(1, (int)items.size());
    auto item = items.at(0);
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::REINSTALL, item->get_action());
}


static std::shared_ptr<Package> nevraToPackage(Transaction & trans, std::string nevra) {
    libdnf5::rpm::Nevra nevraObject;
    if (!nevraObject.parse(nevra.c_str(), libdnf5::rpm::Nevra::Form::NEVRA)) {
        return nullptr;
    }
    if (nevraObject.get_epoch().empty()) {
        nevraObject.set_epoch("0");
    }

    auto rpm = std::make_shared<Package>(trans);
    libdnf5::rpm::copy_nevra_attributes(nevraObject, *rpm);
    return rpm;
}

/*
static TransactionPtr
createTrans(libdnf5::utils::SQLite3Ptr conn, std::string nevra, std::string repoid, TransactionItemAction action, TransactionItemReason reason, std::vector<std::string> obsoletes)
{
    Nevra nevraObject;
    if (!nevraObject.parse(nevra.c_str(), HY_FORM_NEVRA)) {
        return nullptr;
    }
    if (nevraObject.getEpoch() < 0) {
        nevraObject.setEpoch(0);
    }

    Transaction trans(conn);
    auto rpm = nevraToPackage(conn, nevra);

    //std::string repoid = "";
    //TransactionItemReason reason = TransactionItemReason::USER;
    auto ti = trans.addItem(rpm, repoid, action, reason);
    for (auto obs : obsoletes) {
        //ti->addReplacedBy(obs);
    }
    return trans;
}

static TransactionPtr
createTrans(libdnf5::utils::SQLite3Ptr conn, std::string nevra, std::string repoid, TransactionItemAction action, TransactionItemReason reason)
{
    return createTrans(conn, nevra, repoid, action, reason, {});
}
*/

void MergedTransactionTest::test_add_remove_installed() {
    /*
    def test_add_erase_installed(self):
        """Test add with an erasure of NEVRA which was installed before."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Install', 'tour-0:4.6-1.noarch', obsoleted_nevras=('lotus-0:3-16.x86_64',))
        ops.add('Erase', 'tour-0:4.6-1.noarch')

        self.assertCountEqual(
            ops,
            (('Erase', 'lotus-0:3-16.x86_64', None, set()),))
    */

    Transaction trans1(*conn);
    auto trans1_tour = trans1.addItem(
        nevraToPackage(trans1, "tour-0:4.6-1.noarch"),
        "repo1",
        TransactionItemAction::INSTALL,
        TransactionItemReason::USER);

    Transaction trans2(*conn);
    auto trans2_tour = trans2.addItem(
        nevraToPackage(trans2, "tour-0:4.6-1.noarch"),
        "repo2",
        TransactionItemAction::REMOVE,
        TransactionItemReason::DEPENDENCY);

    MergedTransaction merged(trans1);
    merged.merge(trans2);

    auto items = merged.getItems();
    //CPPUNIT_ASSERT_EQUAL(1, (int)items.size());
    // TODO
}

void MergedTransactionTest::test_add_remove_removed() {
    return;
    /*
    def test_add_erase_removed(self):
        """Test add with an erasure of NEVRA which was removed before."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Erase', 'tour-0:4.6-1.noarch')

        self.assertRaises(
            ValueError,
            ops.add, 'Erase', 'tour-0:4.6-1.noarch')
    */

    Transaction trans1(*conn);
    auto trans1_tour = trans1.addItem(
        nevraToPackage(trans1, "tour-0:4.6-1.noarch"),
        "repo1",
        TransactionItemAction::REMOVE,
        TransactionItemReason::USER);

    Transaction trans2(*conn);
    auto trans2_tour = trans2.addItem(
        nevraToPackage(trans2, "tour-0:4.6-1.noarch"),
        "repo2",
        TransactionItemAction::REMOVE,
        TransactionItemReason::DEPENDENCY);

    MergedTransaction merged(trans1);
    merged.merge(trans2);

    auto items = merged.getItems();
    CPPUNIT_ASSERT_EQUAL(1, (int)items.size());

    // TODO: different reasons, repos
}

void MergedTransactionTest::test_add_install_installed() {
    return;
    /*
    def test_add_install_installed(self):
        """Test add with two installs of the same NEVRA."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Install', 'tour-0:4.6-1.noarch')

        self.assertRaises(
            ValueError,
            ops.add, 'Install', 'tour-0:4.6-1.noarch')
    */

    Transaction trans1(*conn);
    auto trans1_tour = trans1.addItem(
        nevraToPackage(trans1, "tour-0:4.6-1.noarch"),
        "repo1",
        TransactionItemAction::INSTALL,
        TransactionItemReason::USER);

    Transaction trans2(*conn);
    auto trans2_tour = trans2.addItem(
        nevraToPackage(trans2, "tour-0:4.6-1.noarch"),
        "repo2",
        TransactionItemAction::INSTALL,
        TransactionItemReason::GROUP);

    MergedTransaction merged(trans1);
    merged.merge(trans2);

    auto items = merged.getItems();
    CPPUNIT_ASSERT_EQUAL(1, (int)items.size());
}

void MergedTransactionTest::test_add_install_removed() {
    return;
    /*
    def test_add_install_removed(self):
        """Test add with an install of NEVRA which was removed before."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Erase', 'tour-0:4.6-1.noarch')
        ops.add('Install', 'tour-0:4.6-1.noarch')

        self.assertCountEqual(
            ops,
            (('Reinstall', 'tour-0:4.6-1.noarch', 'tour-0:4.6-1.noarch', set()),))

    */
    Transaction trans1(*conn);
    auto trans1_tour = trans1.addItem(
        nevraToPackage(trans1, "tour-0:4.6-1.noarch"),
        "repo1",
        TransactionItemAction::REMOVE,
        TransactionItemReason::DEPENDENCY);

    Transaction trans2(*conn);
    auto trans2_tour = trans2.addItem(
        nevraToPackage(trans2, "tour-0:4.6-1.noarch"),
        "repo2",
        TransactionItemAction::INSTALL,
        TransactionItemReason::USER);

    MergedTransaction merged(trans1);
    merged.merge(trans2);

    auto items = merged.getItems();
    CPPUNIT_ASSERT_EQUAL(1, (int)items.size());

    auto item = items.at(0);
    CPPUNIT_ASSERT_EQUAL(std::string("repo2"), item->get_repoid());
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::REINSTALL, item->get_action());
    CPPUNIT_ASSERT_EQUAL(TransactionItemReason::USER, item->get_reason());
}

void MergedTransactionTest::test_add_obsoleted_installed() {
    return;
    /*
    def test_add_obsoleted_installed(self):
        """Test add with an obsoleted NEVRA which was installed before."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Install', 'lotus-0:3-16.x86_64')
        ops.add('Install', 'tour-0:4.6-1.noarch', obsoleted_nevras=('lotus-0:3-16.x86_64',))

        self.assertCountEqual(
            ops,
            (('Install', 'tour-0:4.6-1.noarch', None, set()),))
    */

    Transaction trans1(*conn);
    auto trans1_lotus = trans1.addItem(
        nevraToPackage(trans1, "lotus-0:3-16.x86_64"),
        "repo1",
        TransactionItemAction::INSTALL,
        TransactionItemReason::DEPENDENCY);

    Transaction trans2(*conn);
    auto trans2_tour = trans2.addItem(
        nevraToPackage(trans2, "tour-0:4.6-1.noarch"),
        "repo2",
        TransactionItemAction::INSTALL,
        TransactionItemReason::USER);
    auto trans2_lotus = trans2.addItem(
        nevraToPackage(trans2, "lotus-0:3-16.x86_64"),
        "repo1",
        TransactionItemAction::OBSOLETED,
        TransactionItemReason::DEPENDENCY);

    trans2_lotus->addReplacedBy(trans2_tour);

    MergedTransaction merged(trans1);
    merged.merge(trans2);

    auto items = merged.getItems();
    CPPUNIT_ASSERT_EQUAL(1, (int)items.size());

    auto item = items.at(0);
    CPPUNIT_ASSERT_EQUAL(std::string("repo2"), item->get_repoid());
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::INSTALL, item->get_action());
    CPPUNIT_ASSERT_EQUAL(TransactionItemReason::USER, item->get_reason());
}

void MergedTransactionTest::test_add_obsoleted_obsoleted() {
    return;
    /*
    def test_add_obsoleted_obsoleted(self):
        """Test add with an obsoleted NEVRA which was obsoleted before."""
        ops = dnf.history.NEVRAOperations()
        ops.add(
            'Install',
            'tour-0:4.6-1.noarch',
            obsoleted_nevras=('lotus-0:3-16.x86_64', 'mrkite-0:2-0.x86_64')
        )
        ops.add(
            'Install',
            'pepper-0:20-0.x86_64',
            obsoleted_nevras=('lotus-0:3-16.x86_64', 'librita-0:1-1.x86_64')
        )

        self.assertCountEqual(
            ops,
            (
                (
                    'Install',
                    'tour-0:4.6-1.noarch',
                    None,
                    {'lotus-0:3-16.x86_64', 'mrkite-0:2-0.x86_64'}
                ),
                (
                    'Install',
                    'pepper-0:20-0.x86_64',
                    None,
                    {'lotus-0:3-16.x86_64', 'librita-0:1-1.x86_64'}
                )
            )
        )
    */

    Transaction trans1(*conn);
    auto trans1_tour = trans1.addItem(
        nevraToPackage(trans1, "tour-0:4.6-1.noarch"),
        "repo1",
        TransactionItemAction::INSTALL,
        TransactionItemReason::DEPENDENCY);

    /*
    Transaction trans2(conn);
    auto trans2_tour = trans2.addItem(
        nevraToPackage(conn, "tour-0:4.6-1.noarch"),
        "repo2",
        TransactionItemAction::INSTALL,
        TransactionItemReason::USER
    );
    auto trans2_lotus = trans2.addItem(nevraToPackage(conn, "lotus-0:3-16.x86_64"), "repo1", TransactionItemAction::OBSOLETED, TransactionItemReason::DEPENDENCY);

    trans2_lotus->addReplacedBy(trans2_tour);
    */

    MergedTransaction merged(trans1);
    //merged.merge(trans2);

    auto items = merged.getItems();
    CPPUNIT_ASSERT_EQUAL(1, (int)items.size());

    auto item = items.at(0);
    CPPUNIT_ASSERT_EQUAL(
        std::string("tour-4.6-1.noarch"), std::dynamic_pointer_cast<Package>(item->getItem())->toStr());
    CPPUNIT_ASSERT_EQUAL(std::string("repo1"), item->get_repoid());
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::INSTALL, item->get_action());
    CPPUNIT_ASSERT_EQUAL(TransactionItemReason::DEPENDENCY, item->get_reason());
}

void MergedTransactionTest::test_downgrade() {
    Transaction trans1(*conn);
    trans1.addItem(
        nevraToPackage(trans1, "tour-0:4.6-1.noarch"),
        "repo2",
        TransactionItemAction::DOWNGRADE,
        TransactionItemReason::USER);
    trans1.addItem(
        nevraToPackage(trans1, "tour-0:4.8-1.noarch"),
        "repo1",
        TransactionItemAction::DOWNGRADED,
        TransactionItemReason::USER);

    MergedTransaction merged(trans1);

    auto items = merged.getItems();
    CPPUNIT_ASSERT_EQUAL(2, (int)items.size());

    {
        auto item = items.at(0);
        CPPUNIT_ASSERT_EQUAL(std::string("tour-4.8-1.noarch"), item->getItem()->toStr());
        CPPUNIT_ASSERT_EQUAL(std::string("repo1"), item->get_repoid());
        CPPUNIT_ASSERT_EQUAL(TransactionItemAction::DOWNGRADED, item->get_action());
        CPPUNIT_ASSERT_EQUAL(TransactionItemReason::USER, item->get_reason());
    }

    {
        auto item = items.at(1);
        CPPUNIT_ASSERT_EQUAL(std::string("tour-4.6-1.noarch"), item->getItem()->toStr());
        CPPUNIT_ASSERT_EQUAL(std::string("repo2"), item->get_repoid());
        CPPUNIT_ASSERT_EQUAL(TransactionItemAction::DOWNGRADE, item->get_action());
        CPPUNIT_ASSERT_EQUAL(TransactionItemReason::USER, item->get_reason());
    }
}

void MergedTransactionTest::test_install_downgrade() {
    Transaction trans1(*conn);
    trans1.addItem(
        nevraToPackage(trans1, "tour-0:4.8-1.noarch"),
        "repo1",
        TransactionItemAction::INSTALL,
        TransactionItemReason::USER);

    Transaction trans2(*conn);
    trans2.addItem(
        nevraToPackage(trans2, "tour-0:4.6-1.noarch"),
        "repo2",
        TransactionItemAction::DOWNGRADE,
        TransactionItemReason::USER);
    trans2.addItem(
        nevraToPackage(trans2, "tour-0:4.8-1.noarch"),
        "repo1",
        TransactionItemAction::DOWNGRADED,
        TransactionItemReason::USER);

    MergedTransaction merged(trans1);
    merged.merge(trans2);

    auto items = merged.getItems();
    CPPUNIT_ASSERT_EQUAL(1, (int)items.size());

    auto item = items.at(0);
    CPPUNIT_ASSERT_EQUAL(std::string("tour-4.6-1.noarch"), item->getItem()->toStr());
    CPPUNIT_ASSERT_EQUAL(std::string("repo2"), item->get_repoid());
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::INSTALL, item->get_action());
    CPPUNIT_ASSERT_EQUAL(TransactionItemReason::USER, item->get_reason());
}

void MergedTransactionTest::test_multilib_identity() {
    Transaction trans(*conn);
    trans.addItem(
        nevraToPackage(trans, "gtk3-3.24.8-1.fc30.i686"),
        "repo2",
        TransactionItemAction::DOWNGRADE,
        TransactionItemReason::USER);
    trans.addItem(
        nevraToPackage(trans, "gtk3-3.24.10-1.fc31.i686"),
        "repo2",
        TransactionItemAction::DOWNGRADED,
        TransactionItemReason::USER);
    trans.addItem(
        nevraToPackage(trans, "gtk3-3.24.8-1.fc30.x86_64"),
        "repo2",
        TransactionItemAction::DOWNGRADE,
        TransactionItemReason::USER);
    trans.addItem(
        nevraToPackage(trans, "gtk3-3.24.10-1.fc31.x86_64"),
        "repo2",
        TransactionItemAction::DOWNGRADED,
        TransactionItemReason::USER);

    MergedTransaction merged(trans);

    auto items = merged.getItems();
    CPPUNIT_ASSERT_EQUAL(4, (int)items.size());

    auto item0 = items.at(0);
    CPPUNIT_ASSERT_EQUAL(std::string("gtk3-3.24.10-1.fc31.i686"), item0->getItem()->toStr());
    CPPUNIT_ASSERT_EQUAL(std::string("repo2"), item0->get_repoid());
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::DOWNGRADED, item0->get_action());
    CPPUNIT_ASSERT_EQUAL(TransactionItemReason::USER, item0->get_reason());

    auto item1 = items.at(1);
    CPPUNIT_ASSERT_EQUAL(std::string("gtk3-3.24.8-1.fc30.i686"), item1->getItem()->toStr());
    CPPUNIT_ASSERT_EQUAL(std::string("repo2"), item1->get_repoid());
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::DOWNGRADE, item1->get_action());
    CPPUNIT_ASSERT_EQUAL(TransactionItemReason::USER, item1->get_reason());

    auto item2 = items.at(2);
    CPPUNIT_ASSERT_EQUAL(std::string("gtk3-3.24.10-1.fc31.x86_64"), item2->getItem()->toStr());
    CPPUNIT_ASSERT_EQUAL(std::string("repo2"), item2->get_repoid());
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::DOWNGRADED, item2->get_action());
    CPPUNIT_ASSERT_EQUAL(TransactionItemReason::USER, item2->get_reason());

    auto item3 = items.at(3);
    CPPUNIT_ASSERT_EQUAL(std::string("gtk3-3.24.8-1.fc30.x86_64"), item3->getItem()->toStr());
    CPPUNIT_ASSERT_EQUAL(std::string("repo2"), item3->get_repoid());
    CPPUNIT_ASSERT_EQUAL(TransactionItemAction::DOWNGRADE, item3->get_action());
    CPPUNIT_ASSERT_EQUAL(TransactionItemReason::USER, item3->get_reason());
}

/*
    def test_add_obsoleted_removed(self):
        """Test add with an obsoleted NEVRA which was removed before."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Erase', 'lotus-0:3-16.x86_64')

        self.assertRaises(
            ValueError,
            ops.add, 'Install', 'tour-0:4.6-1.noarch', obsoleted_nevras=('lotus-0:3-16.x86_64',))

    def test_add_reinstall_installed(self):
        """Test add with a reinstall of NEVRA which was installed before."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Install', 'tour-0:4.6-1.noarch')
        ops.add('Reinstall', 'tour-0:4.6-1.noarch', 'tour-0:4.6-1.noarch')

        self.assertCountEqual(
            ops,
            (('Install', 'tour-0:4.6-1.noarch', None, set()),))

    def test_add_replace_installed(self):
        """Test add with a replacing NEVRA which was installed before."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Install', 'tour-0:4.8-1.noarch')

        self.assertRaises(
            ValueError,
            ops.add, 'Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')

    def test_add_replace_opposite(self):
        """Test add with a replacement which was done before, but swapped."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Downgrade', 'tour-0:4.6-1.noarch', 'tour-0:4.8-1.noarch')
        ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')

        self.assertCountEqual(
            ops,
            (('Reinstall', 'tour-0:4.8-1.noarch', 'tour-0:4.8-1.noarch', set()),))

    def test_add_replace_opposite_manual(self):
        """Test add with a manual replacement which was done before, but swapped."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Erase', 'tour-0:4.8-1.noarch')
        ops.add('Install', 'tour-0:4.6-1.noarch')
        ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')

        self.assertCountEqual(
            ops,
            (('Reinstall', 'tour-0:4.8-1.noarch', 'tour-0:4.8-1.noarch', set()),))

    def test_add_replace_removed(self):
        """Test add with a replacing NEVRA which was removed before."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Erase', 'tour-0:4.8-1.noarch')
        ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')

        self.assertCountEqual(
            ops,
            (('Reinstall', 'tour-0:4.8-1.noarch', 'tour-0:4.8-1.noarch', set()),
             ('Erase', 'tour-0:4.6-1.noarch', None, set())))

    def test_add_replaced_opposite(self):
        """Test add with a replaced NEVRA which replaced a NEVRA before in the opposite direction."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Downgrade', 'tour-0:4.6-1.noarch', 'tour-0:4.9-1.noarch')
        ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')

        self.assertCountEqual(
            ops,
            (('Erase', 'tour-0:4.9-1.noarch', None, set()),
             ('Install', 'tour-0:4.8-1.noarch', None, set())))

    def test_add_replaced_removed(self):
        """Test add with a replaced NEVRA which was removed before."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Erase', 'tour-0:4.6-1.noarch')

        self.assertRaises(
            ValueError,
            ops.add, 'Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')

    def test_add_replaced_reinstall(self):
        """Test add with a replaced NEVRA which was reinstalled before."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Reinstall', 'tour-0:4.6-1.noarch', 'tour-0:4.6-1.noarch')
        ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')

        self.assertCountEqual(
            ops,
            (('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch', set()),))

    def test_add_replaced_replacement(self):
        """Test add with a replaced NEVRA which replaced a NEVRA before."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')
        ops.add('Update', 'tour-0:4.9-1.noarch', 'tour-0:4.8-1.noarch')

        self.assertCountEqual(
            ops,
            (('Update', 'tour-0:4.9-1.noarch', 'tour-0:4.6-1.noarch', set()),))

    def test_addition(self):
        """Test addition of two instances."""
        left_ops = dnf.history.NEVRAOperations()
        left_ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')
        right_ops = dnf.history.NEVRAOperations()
        right_ops.add('Update', 'tour-0:4.9-1.noarch', 'tour-0:4.8-1.noarch')
        expected_ops = dnf.history.NEVRAOperations()
        expected_ops.add('Update', 'tour-0:4.9-1.noarch', 'tour-0:4.6-1.noarch')

        result_ops = left_ops + right_ops

        self.assertEqual(result_ops, expected_ops)

    def test_addition_inplace(self):
        """Test in-place addition of two instances."""
        left_ops = dnf.history.NEVRAOperations()
        left_ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')
        right_ops = dnf.history.NEVRAOperations()
        right_ops.add('Update', 'tour-0:4.9-1.noarch', 'tour-0:4.8-1.noarch')
        expected_ops = dnf.history.NEVRAOperations()
        expected_ops.add('Update', 'tour-0:4.9-1.noarch', 'tour-0:4.6-1.noarch')

        left_ops += right_ops

        self.assertEqual(left_ops, expected_ops)

    def test_equality(self):
        """Test equality of two equal instances."""
        left_ops = dnf.history.NEVRAOperations()
        left_ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')
        right_ops = dnf.history.NEVRAOperations()
        right_ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')

        is_equal = left_ops == right_ops

        self.assertTrue(is_equal)

    def test_equality_differentcontent(self):
        """Test equality of two instances with different contents."""
        left_ops = dnf.history.NEVRAOperations()
        left_ops.add('Downgrade', 'tour-0:4.6-1.noarch', 'tour-0:4.8-1.noarch')
        right_ops = dnf.history.NEVRAOperations()
        right_ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')

        is_equal = left_ops == right_ops

        self.assertFalse(is_equal)

    def test_equality_differentlength(self):
        """Test equality of two instances with different lengths."""
        left_ops = dnf.history.NEVRAOperations()
        right_ops = dnf.history.NEVRAOperations()
        right_ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')

        is_equal = left_ops == right_ops

        self.assertFalse(is_equal)

    def test_equality_differenttype(self):
        """Test equality of an instance and an object of a different type."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')

        is_equal = ops == 'tour-0:4.8-1.noarch'

        self.assertFalse(is_equal)

    def test_equality_identity(self):
        """Test equality of the same instance."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')

        is_equal = ops == ops

        self.assertTrue(is_equal)

    def test_inequality(self):
        """Test inequality of two different instances."""
        left_ops = dnf.history.NEVRAOperations()
        left_ops.add('Downgrade', 'tour-0:4.6-1.noarch', 'tour-0:4.8-1.noarch')
        right_ops = dnf.history.NEVRAOperations()
        right_ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')

        is_inequal = left_ops != right_ops

        self.assertTrue(is_inequal)

    def test_inequality_equal(self):
        """Test inequality of two equal instances."""
        left_ops = dnf.history.NEVRAOperations()
        left_ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')
        right_ops = dnf.history.NEVRAOperations()
        right_ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')

        is_inequal = left_ops != right_ops

        self.assertFalse(is_inequal)

    def test_iterator(self):
        """Test iterator of an instance."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')

        iterator = iter(ops)

        self.assertEqual(
            next(iterator),
            ('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch', set()))
        self.assertRaises(StopIteration, next, iterator)

    def test_length(self):
        """Test length of an instance."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Update', 'tour-0:4.8-1.noarch', 'tour-0:4.6-1.noarch')

        length = len(ops)

        self.assertEqual(length, 1)

    def test_membership(self):
        """Test membership of a contained operation."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Update', 'tour-0:4.9-1.noarch', 'tour-0:4.8-1.noarch')

        is_in = ('Update', 'tour-0:4.9-1.noarch', 'tour-0:4.8-1.noarch', ()) in ops

        self.assertTrue(is_in)

    def test_membership_differentnevra(self):
        """Test membership of an operation with different (replacing) NEVRA."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Update', 'tour-0:4.9-1.noarch', 'tour-0:4.8-1.noarch')

        is_in = ('Update', 'pepper-0:20-0.x86_64', 'tour-0:4.8-1.noarch', ()) in ops

        self.assertFalse(is_in)

    def test_membership_differentobsoleted(self):
        """Test membership of an operation with different obsoleted NEVRAs."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Update', 'tour-0:4.9-1.noarch', 'tour-0:4.8-1.noarch')
        op = (
            'Update',
            'tour-0:4.9-1.noarch',
            'tour-0:4.8-1.noarch',
            ('pepper-0:20-0.x86_64',)
        )
        self.assertFalse(op in ops)

    def test_membership_differentreplaced(self):
        """Test membership of an operation with different replaced NEVRA."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Update', 'tour-0:4.9-1.noarch', 'tour-0:4.8-1.noarch')

        is_in = ('Update', 'tour-0:4.9-1.noarch', 'pepper-0:20-0.x86_64', ()) in ops

        self.assertFalse(is_in)

    def test_membership_differentstate(self):
        """Test membership of an operation with different state."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Update', 'tour-0:4.9-1.noarch', 'tour-0:4.8-1.noarch')

        is_in = ('Downgrade', 'tour-0:4.9-1.noarch', 'tour-0:4.8-1.noarch', ()) in ops

        self.assertFalse(is_in)

    def test_membership_differenttype(self):
        """Test membership of an object of a different type."""
        ops = dnf.history.NEVRAOperations()
        ops.add('Update', 'tour-0:4.9-1.noarch', 'tour-0:4.8-1.noarch')

        is_in = 'tour-0:4.9-1.noarch' in ops

        self.assertFalse(is_in)
*/

#endif
