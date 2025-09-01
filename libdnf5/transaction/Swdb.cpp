// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

// TODO(dmach): probably remove most of the functionality and/or merge it into TransactionSack
// the whole file is disabled via the SKIP macro because it doesn't compile with the new code
#ifdef SKIP

#include <cstdio>
#include <filesystem>

//#include "../hy-query-private.hpp"
//#include "../hy-subject.h"
#include "libdnf5/rpm/nevra.hpp"
#include "libdnf5/rpm/package_set.hpp"
#include "libdnf5/utils/bgettext/bgettext-lib.h"
//#include "../utils/filesystem.hpp"
#include "Swdb.hpp"
#include "Transformer.hpp"
#include "rpm_package.hpp"
#include "utils/sqlite3/sqlite3.hpp"

#include "libdnf5/rpm/package_sack_impl.hpp"

namespace libdnf5::transaction {

Swdb::Swdb(libdnf5::utils::SQLite3 & conn) : conn{&conn}, autoClose(true) {}

Swdb::Swdb(libdnf5::utils::SQLite3 & conn, bool autoClose) : conn{&conn}, autoClose(autoClose) {}

Swdb::Swdb(const std::string & path) : conn(nullptr), autoClose(true) {
    if (path == ":memory:") {
        // writing to an in-memory database
        conn = new libdnf5::utils::SQLite3(path);
        Transformer::createDatabase(*conn);
    } else if (!std::filesystem::exists(path.c_str())) {
        // writing to a file that doesn't exist and must be created

        // extract persistdir from path - "/var/lib/dnf/"
        auto found = path.find_last_of("/");

        Transformer transformer(path.substr(0, found), path);
        transformer.transform();

        conn = new libdnf5::utils::SQLite3(path);
    } else {
        // writing to an existing file
        conn = new libdnf5::utils::SQLite3(path);
    }
}

void Swdb::resetDatabase() {
    conn->close();
    if (std::filesystem::exists(getPath().c_str())) {
        remove(getPath().c_str());
    }
    conn->open();
    Transformer::createDatabase(get_connection());
}

void Swdb::closeDatabase() {
    conn->close();
}

Swdb::~Swdb() {
    if (autoClose) {
        try {
            closeDatabase();
        } catch (const std::exception &) {
        }
    }
}

void Swdb::initTransaction() {
    if (transactionInProgress) {
        throw std::logic_error(_("In progress"));
    }
    transactionInProgress = std::unique_ptr<Transaction>(new Transaction(get_connection()));
    itemsInProgress.clear();
}

int64_t Swdb::beginTransaction(int64_t dtStart, std::string rpmdbVersionBegin, std::string cmdline, uint32_t userId) {
    if (!transactionInProgress) {
        throw std::logic_error(_("Not in progress"));
    }

    // begin transaction
    transactionInProgress->set_dt_start(dtStart);
    transactionInProgress->set_rpmdb_version_begin(rpmdbVersionBegin);
    transactionInProgress->set_cmdline(cmdline);
    transactionInProgress->set_user_id(userId);
    transactionInProgress->begin();

    /*
    // save rpm items to map to resolve RPM callbacks
    for (auto item : transactionInProgress->getItems()) {
        auto transItem = item->getItem();
        if (transItem->getItemType() != TransactionItemType::RPM) {
            continue;
        }
        auto rpmItem = std::dynamic_pointer_cast< Package >(transItem);
        itemsInProgress[rpmItem->toStr()] = item;
    }
    */

    return transactionInProgress->get_id();
}

int64_t Swdb::endTransaction(int64_t dtEnd, std::string rpmdbVersionEnd, TransactionState state) {
    if (!transactionInProgress) {
        throw std::logic_error(_("Not in progress"));
    }
    transactionInProgress->set_dt_end(dtEnd);
    transactionInProgress->set_rpmdb_version_end(rpmdbVersionEnd);
    transactionInProgress->finish(state);
    return transactionInProgress->get_id();
}

int64_t Swdb::closeTransaction() {
    if (!transactionInProgress) {
        throw std::logic_error(_("Not in progress"));
    }
    int64_t result = transactionInProgress->get_id();
    transactionInProgress = std::unique_ptr<Transaction>(nullptr);
    itemsInProgress.clear();
    return result;
}


TransactionItemPtr Swdb::addItem(
    std::shared_ptr<Item> item, const std::string & repoid, TransactionItemAction action, TransactionItemReason reason)
//            std::shared_ptr<TransactionItem> replacedBy)
{
    if (!transactionInProgress) {
        throw std::logic_error(_("Not in progress"));
    }
    // auto replacedBy = std::make_shared<TransactionItem>(nullptr);
    return transactionInProgress->addItem(item, repoid, action, reason);
}

void Swdb::setItemDone(const std::string & nevra) {
    if (!transactionInProgress) {
        throw std::logic_error(_("No transaction in progress"));
    }
    auto item = itemsInProgress[nevra];
    item->set_state(TransactionItemState::DONE);
    //item->saveState();
}

TransactionItemReason Swdb::resolveRPMTransactionItemReason(
    const std::string & name, const std::string & arch, int64_t maxTransactionId) {
    // TODO:
    // -1: latest
    // -2: latest and lastTransaction data in memory
    /*
    if (maxTransactionId == -2 && transactionInProgress != nullptr) {
        for (auto i : transactionInProgress->getItems()) {
            auto rpm = std::dynamic_pointer_cast< Package >(i->getItem());
            if (!rpm) {
                continue;
            }
            if (rpm->get_name() == name && rpm->get_arch() == arch) {
                return i->get_reason();
            }
        }
    }
    */

    return Package::resolveTransactionItemReason(get_connection(), name, arch, maxTransactionId);
}

const std::string Swdb::getRPMRepo(const std::string & nevra) {
    libdnf5::rpm::Nevra nevraObject;
    if (!nevraObject.parse(nevra.c_str(), libdnf5::rpm::Nevra::Form::NEVRA)) {
        return "";
    }
    // TODO: hy_nevra_possibility should set epoch to 0 if epoch is not specified
    // and libdnf5::rpm::Nevra::Form::NEVRA is used
    if (nevraObject.get_epoch().empty()) {
        nevraObject.set_epoch(0);
    }

    const char * sql = R"**(
        SELECT
            repo.repoid as repoid
        FROM
            trans_item ti
        JOIN
            rpm USING (item_id)
        JOIN
            repo ON ti.repo_id == repo.id
        WHERE
            ti.action not in (3, 5, 7, 10)
            AND rpm.name = ?
            AND rpm.epoch = ?
            AND rpm.version = ?
            AND rpm.release = ?
            AND rpm.arch = ?
        ORDER BY
            ti.id DESC
        LIMIT 1;
    )**";
    // TODO: where trans.done != 0
    libdnf5::utils::SQLite3::Query query(*conn, sql);
    query.bindv(
        nevraObject.get_name(),
        nevraObject.get_epoch(),
        nevraObject.get_version(),
        nevraObject.get_release(),
        nevraObject.get_arch());
    if (query.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
        auto repoid = query.get<std::string>("repoid");
        return repoid;
    }
    return "";
}

TransactionPtr Swdb::getLastTransaction() {
    const char * sql = R"**(
        SELECT
            id
        FROM
            trans
        ORDER BY
            id DESC
        LIMIT 1
    )**";
    libdnf5::utils::SQLite3::Statement query(get_connection(), sql);
    if (query.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
        auto transId = query.get<int64_t>(0);
        auto transaction = std::make_shared<Transaction>(get_connection(), transId);
        return transaction;
    }
    return nullptr;
}

std::vector<TransactionPtr> Swdb::listTransactions() {
    const char * sql = R"**(
        SELECT
            id
        FROM
            trans
        ORDER BY
            id
    )**";
    libdnf5::utils::SQLite3::Statement query(get_connection(), sql);
    std::vector<TransactionPtr> result;
    while (query.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
        auto transId = query.get<int64_t>(0);
        auto transaction = std::make_shared<Transaction>(get_connection(), transId);
        result.push_back(transaction);
    }
    return result;
}

void Swdb::setReleasever(std::string value) {
    if (!transactionInProgress) {
        throw std::logic_error(_("Not in progress"));
    }
    transactionInProgress->set_releasever(value);
}


void Swdb::add_console_output_line(int file_descriptor, const std::string & line) {
    if (!transactionInProgress) {
        throw std::logic_error(_("Not in progress"));
    }
    transactionInProgress->add_console_output_line(file_descriptor, line);
}

/*
std::vector< TransactionItemPtr >
Swdb::getCompsGroupItemsByPattern(const std::string &pattern)
{
    return CompsGroupItem::getTransactionItemsByPattern(conn, pattern);
}
*/

std::vector<std::string> Swdb::getPackageCompsGroups(const std::string & packageName) {
    const char * sql_all_groups = R"**(
        SELECT DISTINCT
            g.groupid
        FROM
            comps_group g
        JOIN
            comps_group_package p ON p.group_id = g.item_id
        WHERE
            p.name = ?
            AND p.installed = 1
        ORDER BY
            g.groupid
    )**";

    const char * sql_trans_items = R"**(
        SELECT
            ti.action as action,
            ti.reason as reason,
            i.item_id as group_id
        FROM
            trans_item ti
        JOIN
            comps_group i USING (item_id)
        JOIN
            trans t ON ti.trans_id = t.id
        WHERE
            t.state = 1
            AND ti.action not in (3, 5, 7)
            AND i.groupid = ?
        ORDER BY
            ti.trans_id DESC
        LIMIT 1
    )**";

    const char * sql_group_package = R"**(
        SELECT
            p.name
        FROM
            comps_group_package p
        WHERE
            p.group_id = ?
            AND p.installed = 1
    )**";

    std::vector<std::string> result;

    // list all relevant groups
    libdnf5::utils::SQLite3::Query query_all_groups(*conn, sql_all_groups);
    query_all_groups.bindv(packageName);

    while (query_all_groups.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
        auto groupid = query_all_groups.get<std::string>("groupid");
        libdnf5::utils::SQLite3::Query query_trans_items(*conn, sql_trans_items);
        query_trans_items.bindv(groupid);
        if (query_trans_items.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
            auto action = static_cast<TransactionItemAction>(query_trans_items.get<int64_t>("action"));
            // if the last record is group removal, skip
            if (action == TransactionItemAction::REMOVE) {
                continue;
            }
            auto groupId = query_trans_items.get<int64_t>("group_id");
            libdnf5::utils::SQLite3::Query query_group_package(*conn, sql_group_package);
            query_group_package.bindv(groupId);
            if (query_group_package.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
                result.push_back(groupid);
            }
        }
    }
    return result;
}

std::vector<std::string> Swdb::getCompsGroupEnvironments(const std::string & groupId) {
    const char * sql_all_environments = R"**(
        SELECT DISTINCT
            e.environmentid
        FROM
            comps_environment e
        JOIN
            comps_environment_group g ON g.environment_id = e.item_id
        WHERE
            g.groupid = ?
            AND g.installed = 1
        ORDER BY
            e.environmentid
    )**";

    const char * sql_trans_items = R"**(
        SELECT
            ti.action as action,
            ti.reason as reason,
            i.item_id as environment_id
        FROM
            trans_item ti
        JOIN
            comps_environment i USING (item_id)
        JOIN
            trans t ON ti.trans_id = t.id
        WHERE
            t.state = 1
            AND ti.action not in (3, 5, 7)
            AND i.environmentid = ?
        ORDER BY
            ti.trans_id DESC
        LIMIT 1
    )**";

    const char * sql_environment_group = R"**(
        SELECT
            g.groupid
        FROM
            comps_environment_group g
        WHERE
            g.environment_id = ?
            AND g.installed = 1
    )**";

    std::vector<std::string> result;

    // list all relevant groups
    libdnf5::utils::SQLite3::Query query_all_environments(*conn, sql_all_environments);
    query_all_environments.bindv(groupId);

    while (query_all_environments.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
        auto envid = query_all_environments.get<std::string>("environmentid");
        libdnf5::utils::SQLite3::Query query_trans_items(*conn, sql_trans_items);
        query_trans_items.bindv(envid);
        if (query_trans_items.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
            auto action = static_cast<TransactionItemAction>(query_trans_items.get<int64_t>("action"));
            // if the last record is group removal, skip
            if (action == TransactionItemAction::REMOVE) {
                continue;
            }
            auto envId = query_trans_items.get<int64_t>("environment_id");
            libdnf5::utils::SQLite3::Query query_environment_group(*conn, sql_environment_group);
            query_environment_group.bindv(envId);
            if (query_environment_group.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
                result.push_back(envid);
            }
        }
    }
    return result;
}


/*
std::vector< TransactionItemPtr >
Swdb::getCompsEnvironmentItemsByPattern(const std::string &pattern)
{
    return CompsEnvironmentItem::getTransactionItemsByPattern(conn, pattern);
}
*/

/*
PackagePtr
Swdb::createPackage()
{
    return std::make_shared< Package >(conn);
}

CompsEnvironmentItemPtr
Swdb::createCompsEnvironmentItem()
{
    return std::make_shared< CompsEnvironmentItem >(conn);
}

CompsGroupItemPtr
Swdb::createCompsGroupItem()
{
    return std::make_shared< CompsGroupItem >(conn);
}
*/

std::vector<int64_t> Swdb::searchTransactionsByRPM(const std::vector<std::string> & patterns) {
    return Package::searchTransactions(get_connection(), patterns);
}

}  // namespace libdnf5::transaction

#endif
