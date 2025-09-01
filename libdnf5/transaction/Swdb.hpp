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

#ifndef LIBDNF5_TRANSACTION_SWDB_HPP
#define LIBDNF5_TRANSACTION_SWDB_HPP

#include <sys/stat.h>

#include <map>
#include <memory>
#include <vector>

namespace libdnf5::transaction {
struct Swdb;
//class Transformer;
}  // namespace libdnf5::transaction

//#include "../hy-types.h"
//#include "../sack/query.hpp"
#include "comps_group.hpp"
#include "transaction.hpp"
#include "transaction_item.hpp"
#include "utils/sqlite3/sqlite3.hpp"

#include "libdnf5/rpm/package_set.hpp"

namespace libdnf5::transaction {

struct Swdb {
public:
    explicit Swdb(libdnf5::utils::SQLite3 & conn);
    explicit Swdb(const std::string & path);
    ~Swdb();

    // Database
    // FIXME load this from conf
    static constexpr const char * defaultPath = "/var/lib/dnf/history.sqlite";
    static constexpr const char * defaultDatabaseName = "history.sqlite";

    const std::string & getPath() { return conn->get_path(); }
    void resetDatabase();
    void closeDatabase();

    // Transaction in progress
    void initTransaction();
    int64_t beginTransaction(int64_t dtBegin, std::string rpmdbVersionBegin, std::string cmdline, uint32_t userId);
    int64_t endTransaction(int64_t dtEnd, std::string rpmdbVersionEnd, TransactionState state);
    int64_t closeTransaction();
    // TODO:
    //std::vector< TransactionItemPtr > getItems() { return transactionInProgress->getItems(); }

    const std::vector<std::unique_ptr<CompsEnvironment>> & get_comps_environments() const noexcept {
        return transactionInProgress->get_comps_environments();
    }
    //CompsEnvironment & new_comps_environment();

    const std::vector<std::unique_ptr<CompsGroup>> & get_comps_groups() const noexcept {
        return transactionInProgress->get_comps_groups();
    }
    //CompsGroup & new_comps_group();

    const std::vector<std::unique_ptr<Package>> & get_packages() const noexcept {
        return transactionInProgress->get_packages();
    }
    //Package & new_package();

    TransactionPtr getLastTransaction();
    std::vector<TransactionPtr> listTransactions();  // std::vector<long long> transactionIds);

    // TransactionItems
    TransactionItemPtr addItem(
        ItemPtr item, const std::string & repoid, TransactionItemAction action, TransactionItemReason reason);
    // std::shared_ptr<TransactionItem> replacedBy);

    // TODO: remove; TransactionItem states are saved on transaction save
    void setItemDone(const std::string & nevra);

    // Item: constructors
    /*
    PackagePtr createPackage();
    CompsGroupItemPtr createCompsGroupItem();
    CompsEnvironmentItemPtr createCompsEnvironmentItem();
    */

    // Item: RPM
    TransactionItemReason resolveRPMTransactionItemReason(
        const std::string & name, const std::string & arch, int64_t maxTransactionId);
    const std::string getRPMRepo(const std::string & nevra);
    //TransactionItemPtr getRPMTransactionItem(const std::string &nevra);
    std::vector<int64_t> searchTransactionsByRPM(const std::vector<std::string> & patterns);

    // Item: CompsGroup
    TransactionItemPtr getCompsGroupItem(const std::string & groupid);
    std::vector<TransactionItemPtr> getCompsGroupItemsByPattern(const std::string & pattern);
    std::vector<std::string> getPackageCompsGroups(const std::string & packageName);

    // Item: CompsEnvironment
    TransactionItemPtr getCompsEnvironmentItem(const std::string & envid);
    std::vector<TransactionItemPtr> getCompsEnvironmentItemsByPattern(const std::string & pattern);
    std::vector<std::string> getCompsGroupEnvironments(const std::string & groupId);

    // misc
    void setReleasever(std::string value);
    void add_console_output_line(int file_descriptor, const std::string & line);

    libdnf5::utils::SQLite3 & get_connection() const { return *conn; }

    Transaction * get_transaction_in_progress() { return transactionInProgress.get(); }

protected:
    //friend class Transformer;

    explicit Swdb(libdnf5::utils::SQLite3 & conn, bool autoClose);
    libdnf5::utils::SQLite3 * conn;
    bool autoClose;
    std::unique_ptr<Transaction> transactionInProgress = nullptr;
    std::map<std::string, TransactionItemPtr> itemsInProgress;

private:
};

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_SWDB_HPP

#endif
