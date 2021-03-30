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


#include "libdnf/transaction/rpm_package.hpp"

#include "libdnf/rpm/nevra.hpp"
#include "libdnf/transaction/db/rpm.hpp"
#include "libdnf/transaction/transaction.hpp"

#include <algorithm>
#include <map>
#include <sstream>


namespace libdnf::transaction {


Package::Package(Transaction & trans) : TransactionItem::TransactionItem(trans, Type::RPM) {}


uint32_t Package::get_epoch_int() const {
    if (get_epoch().empty()) {
        return 0;
    }
    return static_cast<uint32_t>(std::stoi(get_epoch()));
}


std::string Package::to_string() const {
    return libdnf::rpm::to_full_nevra_string(*this);
}


/*
TransactionItemReason Package::resolveTransactionItemReason(
    libdnf::utils::SQLite3 & conn,
    const std::string & name,
    const std::string & arch,
    [[maybe_unused]] int64_t maxTransactionId) {
    const char * sql = R"**(
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
            auto action = static_cast<TransactionItemAction>(query.get<int64_t>("action"));
            if (action == TransactionItemAction::REMOVE) {
                return TransactionItemReason::UNKNOWN;
            }
            auto reason = static_cast<TransactionItemReason>(query.get<int64_t>("reason"));
            return reason;
        }
    } else {
        const char * arch_sql = R"**(
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
            auto rpm_arch = arch_query.get<std::string>("arch");

            libdnf::utils::SQLite3::Query query(conn, sql);
            query.bindv(name, rpm_arch);
            while (query.step() == libdnf::utils::SQLite3::Statement::StepResult::ROW) {
                auto action = static_cast<TransactionItemAction>(query.get<int64_t>("action"));
                if (action == TransactionItemAction::REMOVE) {
                    continue;
                }
                auto reason = static_cast<TransactionItemReason>(query.get<int64_t>("reason"));
                if (reason > result) {
                    result = reason;
                }
            }
        }
        return result;
    }
    return TransactionItemReason::UNKNOWN;
}
*/

bool Package::operator<(const Package & other) const {
    // TODO(dmach): replace the whole function with a different implementation, preferably an existing code
    // compare epochs
    if (get_epoch_int() < other.get_epoch_int()) {
        return true;
    } else if (get_epoch_int() > other.get_epoch_int()) {
        return false;
    }

    // compare versions
    std::stringstream versionThis(get_version());
    std::stringstream versionOther(other.get_version());

    std::string bufferThis;
    std::string bufferOther;
    while (std::getline(versionThis, bufferThis, '.') && std::getline(versionOther, bufferOther, '.')) {
        int subVersionThis = std::stoi(bufferThis);
        int subVersionOther = std::stoi(bufferOther);
        if (subVersionThis == subVersionOther) {
            continue;
        }
        return subVersionOther > subVersionThis;
    }
    return false;
}

/*
std::vector<int64_t> Package::searchTransactions(
    libdnf::utils::SQLite3 & conn, const std::vector<std::string> & patterns) {
    std::vector<int64_t> result;

    const char * sql = R"**(
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
            result.push_back(query.get<int64_t>("id"));
        }
    }
    std::sort(result.begin(), result.end());
    auto last = std::unique(result.begin(), result.end());
    result.erase(last, result.end());
    return result;
}
*/

}  // namespace libdnf::transaction
