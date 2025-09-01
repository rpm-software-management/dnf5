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


#include "libdnf5/transaction/rpm_package.hpp"

#include "db/rpm.hpp"

#include "libdnf5/rpm/nevra.hpp"
#include "libdnf5/transaction/transaction.hpp"

#include <sstream>


namespace libdnf5::transaction {

class Package::Impl {
private:
    friend Package;
    std::string name;
    std::string epoch;
    std::string version;
    std::string release;
    std::string arch;
};

Package::Package(const Transaction & trans)
    : TransactionItem::TransactionItem(trans),
      p_impl(std::make_unique<Impl>()) {}

Package::Package(const Package & src) : TransactionItem(src), p_impl(new Impl(*src.p_impl)) {}
Package::Package(Package && src) noexcept = default;

Package & Package::operator=(const Package & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
Package & Package::operator=(Package && src) noexcept = default;
Package::~Package() = default;

const std::string & Package::get_name() const noexcept {
    return p_impl->name;
}
const std::string & Package::get_epoch() const noexcept {
    return p_impl->epoch;
}
const std::string & Package::get_release() const noexcept {
    return p_impl->release;
}
const std::string & Package::get_arch() const noexcept {
    return p_impl->arch;
}
const std::string & Package::get_version() const noexcept {
    return p_impl->version;
}

void Package::set_name(const std::string & value) {
    p_impl->name = value;
}
void Package::set_epoch(const std::string & value) {
    p_impl->epoch = value;
}
void Package::set_version(const std::string & value) {
    p_impl->version = value;
}
void Package::set_release(const std::string & value) {
    p_impl->release = value;
}
void Package::set_arch(const std::string & value) {
    p_impl->arch = value;
}

uint32_t Package::get_epoch_int() const {
    if (get_epoch().empty()) {
        return 0;
    }
    return static_cast<uint32_t>(std::stoi(get_epoch()));
}


std::string Package::to_string() const {
    return libdnf5::rpm::to_full_nevra_string(*this);
}


/*
TransactionItemReason Package::resolveTransactionItemReason(
    libdnf5::utils::SQLite3 & conn,
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
        libdnf5::utils::SQLite3::Query query(conn, sql);
        query.bindv(name, arch);

        if (query.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
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

        libdnf5::utils::SQLite3::Query arch_query(conn, arch_sql);
        arch_query.bindv(name);

        TransactionItemReason result = TransactionItemReason::UNKNOWN;

        while (arch_query.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
            auto rpm_arch = arch_query.get<std::string>("arch");

            libdnf5::utils::SQLite3::Query query(conn, sql);
            query.bindv(name, rpm_arch);
            while (query.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
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
    libdnf5::utils::SQLite3 & conn, const std::vector<std::string> & patterns) {
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
    libdnf5::utils::SQLite3::Query query(conn, sql);
    for (auto pattern : patterns) {
        query.bindv(pattern, pattern, pattern, pattern, pattern);
        while (query.step() == libdnf5::utils::SQLite3::Statement::StepResult::ROW) {
            result.push_back(query.get<int64_t>("id"));
        }
    }
    std::sort(result.begin(), result.end());
    auto last = std::unique(result.begin(), result.end());
    result.erase(last, result.end());
    return result;
}
*/

}  // namespace libdnf5::transaction
