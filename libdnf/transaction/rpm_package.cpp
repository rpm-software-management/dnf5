/*
 * Copyright (C) 2017-2018 Red Hat, Inc.
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <algorithm>
#include <map>
#include <sstream>

//#include "../hy-subject.h"
#include "libdnf/rpm/nevra.hpp"
#include "libdnf/transaction/db/rpm.hpp"

#include "transaction.hpp"
#include "rpm_package.hpp"

namespace libdnf::transaction {


void
Package::save()
{
    if (getId() == 0) {
        auto query = rpm_select_pk_new_query(trans.get_connection());
        setId(rpm_select_pk(*query, *this));
        if (!getId()) {
            // populates this->id
            Item::save();
            auto query = rpm_insert_new_query(trans.get_connection());
            rpm_insert(*query, *this);
        }
    } else {
        // TODO: dbUpdate() ?
    }
}


uint32_t Package::get_epoch_int() const {
    if (get_epoch().empty()) {
        return 0;
    }
    return std::stoi(get_epoch());
}


std::string
Package::getNEVRA() const
{
    // TODO: use string formatting
    if (!epoch.empty() && epoch != "0") {
        return name + "-" + epoch + ":" + version + "-" + release + "." + arch;
    }
    return name + "-" + version + "-" + release + "." + arch;
}

std::string
Package::toStr() const
{
    return getNEVRA();
}


/*
TransactionItemPtr
Package::getTransactionItem(libdnf::utils::SQLite3Ptr conn, const std::string &nevra)
{
    libdnf::rpm::Nevra nevraObject;
    if (!nevraObject.parse(nevra.c_str(), libdnf::rpm::Nevra::Form::NEVRA)) {
        return nullptr;
    }
    // TODO: hy_nevra_possibility should set epoch to 0 if epoch is not specified and libdnf::rpm::Nevra::Form::NEVRA
    // is used
    if (nevraObject.get_epoch().empty()) {
        nevraObject.set_epoch("0");
    }

    const char *sql = R"**(
        SELECT
            ti.trans_id,
            ti.id,
            ti.action,
            ti.reason,
            ti.state,
            r.repoid,
            i.item_id,
            i.name,
            i.epoch,
            i.version,
            i.release,
            i.arch
        FROM
            trans_item ti,
            repo r,
            rpm i
        WHERE
            ti.repo_id = r.id
            AND ti.item_id = i.item_id
            AND i.name = ?
            AND i.epoch = ?
            AND i.version = ?
            AND i.release = ?
            AND i.arch = ?
        ORDER BY
           ti.id DESC
        LIMIT 1
    )**";
    libdnf::utils::SQLite3::Query query(*conn, sql);
    query.bindv(nevraObject.get_name(),
                nevraObject.get_epoch(),
                nevraObject.get_version(),
                nevraObject.get_release(),
                nevraObject.get_arch());
    if (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
        return transactionItemFromQuery(conn, query, query.get< int64_t >("trans_id"));
    }
    return nullptr;
}
*/

TransactionItemReason
Package::resolveTransactionItemReason(libdnf::utils::SQLite3 & conn,
                                      const std::string &name,
                                      const std::string &arch,
                                      [[maybe_unused]] int64_t maxTransactionId)
{
    const char *sql = R"**(
        SELECT
            ti.action as action,
            ti.reason as reason
        FROM
            trans_item ti
        JOIN
            trans t ON ti.trans_id = t.id
        JOIN
            rpm i USING (item_id)
        WHERE
            t.state = 1
            /* see comment in TransactionItem.hpp - TransactionItemAction */
            AND ti.action not in (3, 5, 7, 10)
            AND i.name = ?
            AND i.arch = ?
        ORDER BY
            ti.trans_id DESC
        LIMIT 1
    )**";

    if (arch != "") {
        libdnf::utils::SQLite3::Query query(conn, sql);
        query.bindv(name, arch);

        if (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
            auto action = static_cast< TransactionItemAction >(query.get< int64_t >("action"));
            if (action == TransactionItemAction::REMOVE) {
                return TransactionItemReason::UNKNOWN;
            }
            auto reason = static_cast< TransactionItemReason >(query.get< int64_t >("reason"));
            return reason;
        }
    } else {
        const char *arch_sql = R"**(
            SELECT DISTINCT
                arch
            FROM
                rpm
            WHERE
                name = ?
        )**";

        libdnf::utils::SQLite3::Query arch_query(conn, arch_sql);
        arch_query.bindv(name);

        TransactionItemReason result = TransactionItemReason::UNKNOWN;

        while (arch_query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
            auto rpm_arch = arch_query.get< std::string >("arch");

            libdnf::utils::SQLite3::Query query(conn, sql);
            query.bindv(name, rpm_arch);
            while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
                auto action = static_cast< TransactionItemAction >(query.get< int64_t >("action"));
                if (action == TransactionItemAction::REMOVE) {
                    continue;
                }
                auto reason = static_cast< TransactionItemReason >(query.get< int64_t >("reason"));
                if (reason > result) {
                    result = reason;
                }
            }
        }
        return result;
    }
    return TransactionItemReason::UNKNOWN;
}

bool
Package::operator<(const Package &other) const
{
    // TODO(dmach): replace the whole function with a different implementation, preferably an existing code
    // compare epochs
    int32_t epoch_diff = other.get_epoch_int() - get_epoch_int();
    if (epoch_diff > 0) {
        return true;
    } else if (epoch_diff < 0) {
        return false;
    }

    // compare versions
    std::stringstream versionThis( get_version());
    std::stringstream versionOther(other.get_version());

    std::string bufferThis;
    std::string bufferOther;
    while (std::getline(versionThis, bufferThis, '.') &&
           std::getline(versionOther, bufferOther, '.')) {
        int subVersionThis = std::stoi(bufferThis);
        int subVersionOther = std::stoi(bufferOther);
        if (subVersionThis == subVersionOther) {
            continue;
        }
        return subVersionOther > subVersionThis;
    }
    return false;
}

std::vector< int64_t >
Package::searchTransactions(libdnf::utils::SQLite3 & conn, const std::vector< std::string > &patterns)
{
    std::vector< int64_t > result;

    const char *sql = R"**(
        SELECT DISTINCT
            t.id
        FROM
            trans t
        JOIN
            trans_item ti ON ti.trans_id = t.id
        JOIN
            rpm i USING (item_id)
        WHERE
            t.state = 1
            AND (
                i.name = ?
                OR i.epoch = ?
                OR i.version = ?
                OR i.release = ?
                OR i.arch = ?
            )
        ORDER BY
           trans_id DESC
    )**";
    libdnf::utils::SQLite3::Query query(conn, sql);
    for (auto pattern : patterns) {
        query.bindv(pattern, pattern, pattern, pattern, pattern);
        while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
            result.push_back(query.get< int64_t >("id"));
        }
    }
    std::sort(result.begin(), result.end());
    auto last = std::unique(result.begin(), result.end());
    result.erase(last, result.end());
    return result;
}

}  // namespace libdnf::transaction
