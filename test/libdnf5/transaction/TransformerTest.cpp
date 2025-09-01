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

#include "TransformerTest.hpp"

#include <json.h>
#include <libdnf5/transaction/Swdb.hpp>
#include <libdnf5/transaction/rpm_package.hpp>
#include <libdnf5/transaction/transaction.hpp>
#include <libdnf5/transaction/transaction_item.hpp>

#include <set>
#include <sstream>
#include <string>

using namespace libdnf5::transaction;

CPPUNIT_TEST_SUITE_REGISTRATION(TransformerTest);

TransformerMock::TransformerMock() : Transformer("", "") {}

static const char * create_history_sql =
#include "sql/create_test_history_db.sql"
    ;

static const char * groups_json =
#include "assets/groups.json"
    ;

void TransformerTest::setUp() {
    swdb = new libdnf5::utils::SQLite3(":memory:");
    history = new libdnf5::utils::SQLite3(":memory:");
    Transformer::createDatabase(*swdb);
    history->exec(create_history_sql);
}

void TransformerTest::tearDown() {
    delete swdb;
    delete history;
}

void TransformerTest::testGroupTransformation() {
    // load test groups.json
    struct json_object * groupsJson = json_tokener_parse(groups_json);

    // perform the transformation
    transformer.processGroupPersistor(*swdb, groupsJson);

    swdb->backup("db.sql");

    // check basic stuff in generated transaction
    Transaction trans(*swdb, 1);
    CPPUNIT_ASSERT_EQUAL((int64_t)1, trans.get_id());
    CPPUNIT_ASSERT_EQUAL(TransactionState::DONE, trans.get_state());

    // load transaction items
    auto items = trans.getItems();
    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(items.size()));

    // verify items
    for (auto transItem : items) {
        auto item = transItem->getItem();
        auto type = item->getItemType();
        if (type == TransactionItemType::GROUP) {
            auto group = std::dynamic_pointer_cast<CompsGroup>(item);

            CPPUNIT_ASSERT(group->get_group_id() == "core");
            CPPUNIT_ASSERT("Core" == group->get_name());
            CPPUNIT_ASSERT("Úplný základ" == group->get_translated_name());

            auto & packages = group->get_packages();

            CPPUNIT_ASSERT(1 == packages.size());

            auto & groupPkg = packages[0];
            CPPUNIT_ASSERT(groupPkg->get_name() == "dnf-yum");
            CPPUNIT_ASSERT(groupPkg->get_installed() == true);
            CPPUNIT_ASSERT(groupPkg->get_package_type() == PackageType::MANDATORY);

        } else if (type == TransactionItemType::ENVIRONMENT) {
            auto env = std::dynamic_pointer_cast<CompsEnvironment>(item);
            CPPUNIT_ASSERT("minimal-environment" == env->get_environment_id());
            CPPUNIT_ASSERT("Minimal Install" == env->get_name());
            CPPUNIT_ASSERT("Minimálna inštalácia" == env->get_translated_name());

            auto & groups = env->get_groups();
            CPPUNIT_ASSERT(1 == groups.size());

            auto & envGroup = groups[0];
            CPPUNIT_ASSERT(envGroup->get_group_id() == "core");
            CPPUNIT_ASSERT(envGroup->get_installed() == true);
            CPPUNIT_ASSERT(envGroup->get_group_type() == PackageType::MANDATORY);

        } else {
            CPPUNIT_FAIL("Invalid item type: " + std::to_string(static_cast<int>(type)));
        }
    }

    json_object_put(groupsJson);
}

void TransformerTest::testTransformTrans() {
    // perform database transformation
    transformer.transformTrans(*swdb, *history);

    // check first transaction attributes
    Transaction first(*swdb, 1);
    CPPUNIT_ASSERT(first.get_id() == 1);
    CPPUNIT_ASSERT(first.get_dt_begin() == 1513267401);
    CPPUNIT_ASSERT(first.get_dt_end() == 1513267509);
    CPPUNIT_ASSERT(first.get_rpmdb_version_begin() == "2213:9795b6a4db5e5368628b5240ec63a629833c5594");
    CPPUNIT_ASSERT(first.get_rpmdb_version_end() == "2213:9eab991133c166f8bcf3ecea9fb422b853f7aebc");
    CPPUNIT_ASSERT(first.get_releasever() == "26");
    CPPUNIT_ASSERT(first.get_user_id() == 1000);
    CPPUNIT_ASSERT(first.get_cmdline() == "upgrade -y");
    CPPUNIT_ASSERT(first.get_state() == TransactionState::DONE);

    // check first transaction output
    auto firstOut = first.get_console_output();
    CPPUNIT_ASSERT(firstOut.size() == 2);
    CPPUNIT_ASSERT(firstOut[0].first == 1);
    CPPUNIT_ASSERT(firstOut[0].second == "line1");
    CPPUNIT_ASSERT(firstOut[1].first == 1);
    CPPUNIT_ASSERT(firstOut[1].second == "line2");

    // check software performed with
    auto firstSoftWith = first.get_runtime_packages();
    CPPUNIT_ASSERT(firstSoftWith.size() == 1);
    for (auto nevra : firstSoftWith) {
        CPPUNIT_ASSERT_EQUAL(std::string("chrony-1:3.1-4.fc26.x86_64"), nevra);
    }

    // check first transaction items
    auto items = first.getItems();
    CPPUNIT_ASSERT(items.size() == 2);
    for (auto item : items) {
        auto rpm = std::dynamic_pointer_cast<Package>(item->getItem());
        if (rpm->get_name() == "chrony" && rpm->get_version() == "3.1") {
            CPPUNIT_ASSERT(rpm->get_epoch() == "1");
            CPPUNIT_ASSERT(rpm->get_release() == "4.fc26");
            CPPUNIT_ASSERT(item->get_action() == TransactionItemAction::UPGRADED);
            CPPUNIT_ASSERT(item->get_reason() == TransactionItemReason::USER);
            CPPUNIT_ASSERT(item->get_state() == TransactionItemState::DONE);

            // TODO repo, replaced
        } else {  // chrony 3.2
            CPPUNIT_ASSERT(rpm->get_epoch() == "1");
            CPPUNIT_ASSERT(rpm->get_version() == "3.2");
            CPPUNIT_ASSERT(rpm->get_release() == "4.fc26");

            CPPUNIT_ASSERT(item->get_action() == TransactionItemAction::UPGRADE);
            CPPUNIT_ASSERT(item->get_reason() == TransactionItemReason::USER);
            CPPUNIT_ASSERT(item->get_state() == TransactionItemState::DONE);
            // TODO repo, replaced
        }
    }

    // check second transaction attributes
    Transaction second(*swdb, 2);
    CPPUNIT_ASSERT(second.get_id() == 2);
    CPPUNIT_ASSERT(second.get_dt_begin() == 1513267535);
    CPPUNIT_ASSERT(second.get_dt_end() == 1513267539);
    CPPUNIT_ASSERT(second.get_rpmdb_version_begin() == "2213:9eab991133c166f8bcf3ecea9fb422b853f7aebc");
    CPPUNIT_ASSERT(second.get_rpmdb_version_end() == "2214:e02004142740afb5b6d148d50bc84be4ab41ad13");
    CPPUNIT_ASSERT(second.get_user_id() == 1000);
    CPPUNIT_ASSERT(second.get_cmdline() == "-y install Foo");
    CPPUNIT_ASSERT(second.get_state() == TransactionState::DONE);

    // check second transaction console output
    auto secondOut = second.get_console_output();
    CPPUNIT_ASSERT(secondOut.size() == 2);
    CPPUNIT_ASSERT(secondOut[0].first == 2);
    CPPUNIT_ASSERT(secondOut[0].second == "msg1");
    CPPUNIT_ASSERT(secondOut[1].first == 2);
    CPPUNIT_ASSERT(secondOut[1].second == "msg2");

    // check second transaction performed with software
    std::set<std::string> possibleValues = {"chrony-1:3.1-4.fc26.x86_64", "kernel-4.11-301.fc26.x86_64"};
    auto secondSoftWith = second.get_runtime_packages();
    CPPUNIT_ASSERT(secondSoftWith.size() == 2);
    for (auto nevra : secondSoftWith) {
        auto it = possibleValues.find(nevra);
        CPPUNIT_ASSERT(it != possibleValues.end());
        possibleValues.erase(it);
    }

    // check second transaction items
    items = second.getItems();
    CPPUNIT_ASSERT(items.size() == 1);
    for (auto item : items) {
        auto kernel = std::dynamic_pointer_cast<Package>(item->getItem());
        CPPUNIT_ASSERT(kernel->get_name() == "kernel");
        CPPUNIT_ASSERT(kernel->get_epoch() == "0");
        CPPUNIT_ASSERT(kernel->get_version() == "4.11");
        CPPUNIT_ASSERT(kernel->get_release() == "301.fc26");

        CPPUNIT_ASSERT(item->get_action() == TransactionItemAction::INSTALL);
        CPPUNIT_ASSERT(item->get_reason() == TransactionItemReason::DEPENDENCY);
        CPPUNIT_ASSERT(item->get_state() == TransactionItemState::DONE);
    }

    swdb->backup("sql.db");
}

#endif
