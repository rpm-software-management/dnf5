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

#include "Transaction.hpp"
#include "CompsEnvironmentItem.hpp"
#include "CompsGroupItem.hpp"
#include "RPMItem.hpp"
#include "TransactionItem.hpp"

namespace libdnf {

Transaction::Transaction(SQLite3Ptr conn, int64_t pk)
  : conn{conn}
{
    dbSelect(pk);
}

Transaction::Transaction(SQLite3Ptr conn)
  : conn{conn}
{
}

bool
Transaction::operator==(const Transaction &other) const
{
    return getId() == other.getId() && getDtBegin() == other.getDtBegin() &&
           getRpmdbVersionBegin() == other.getRpmdbVersionBegin();
}

/**
 * Compare two transactions on:
 *  transaction ID
 *  begin timestamp
 *  packages in the system at the time of transaction
 * \param other transaction to compare with
 * \return true if other transaction is older
 */
bool
Transaction::operator<(const Transaction &other) const
{
    return getId() > other.getId() || getDtBegin() > other.getDtBegin() ||
           getRpmdbVersionBegin() > other.getRpmdbVersionBegin();
}

/**
 * \param other transaction to compare with
 * \return true if other transaction is newer
 */
bool
Transaction::operator>(const Transaction &other) const
{
    return getId() < other.getId() || getDtBegin() < other.getDtBegin() ||
           getRpmdbVersionBegin() < other.getRpmdbVersionBegin();
}

void
Transaction::dbSelect(int64_t pk)
{
    const char *sql =
        "SELECT "
        "  dt_begin, "
        "  dt_end, "
        "  rpmdb_version_begin, "
        "  rpmdb_version_end, "
        "  releasever, "
        "  user_id, "
        "  cmdline, "
        "  state "
        "FROM "
        "  trans "
        "WHERE "
        "  id = ?";
    SQLite3::Query query(*conn.get(), sql);
    query.bindv(pk);
    query.step();

    id = pk;
    dtBegin = query.get< int >("dt_begin");
    dtEnd = query.get< int >("dt_end");
    rpmdbVersionBegin = query.get< std::string >("rpmdb_version_begin");
    rpmdbVersionEnd = query.get< std::string >("rpmdb_version_end");
    releasever = query.get< std::string >("releasever");
    userId = query.get< uint32_t >("user_id");
    cmdline = query.get< std::string >("cmdline");
    state = static_cast< TransactionState >(query.get< int >("state"));
}

/**
 * Loader for the transaction items.
 * \return list of transaction items associated with the transaction
 */
std::vector< TransactionItemPtr >
Transaction::getItems()
{
    std::vector< TransactionItemPtr > result;
    auto rpms = RPMItem::getTransactionItems(conn, getId());
    result.insert(result.end(), rpms.begin(), rpms.end());

    auto comps_groups = CompsGroupItem::getTransactionItems(conn, getId());
    result.insert(result.end(), comps_groups.begin(), comps_groups.end());

    auto comps_environments = CompsEnvironmentItem::getTransactionItems(conn, getId());
    result.insert(result.end(), comps_environments.begin(), comps_environments.end());

    return result;
}

/**
 * Load list of software performed with for current transaction from the database.
 * Transaction has to be saved in advance, otherwise empty list will be returned.
 * \return list of RPMItem objects that performed the transaction
 */
const std::set< std::shared_ptr< RPMItem > >
Transaction::getSoftwarePerformedWith() const
{
    const char *sql = R"**(
        SELECT
            item_id
        FROM
            trans_with
        WHERE
            trans_id = ?
    )**";

    std::set< std::shared_ptr< RPMItem > > software;

    SQLite3::Query query(*conn.get(), sql);
    query.bindv(getId());

    while (query.step() == SQLite3::Statement::StepResult::ROW) {
        software.insert(std::make_shared< RPMItem >(conn, query.get< int64_t >("item_id")));
    }

    return software;
}

std::vector< std::pair< int, std::string > >
Transaction::getConsoleOutput() const
{
    const char *sql = R"**(
        SELECT
            file_descriptor,
            line
        FROM
            console_output
        WHERE
            trans_id = ?
        ORDER BY
            id
    )**";
    SQLite3::Query query(*conn, sql);
    query.bindv(getId());
    std::vector< std::pair< int, std::string > > result;
    while (query.step() == SQLite3::Statement::StepResult::ROW) {
        auto fileDescriptor = query.get< int >("file_descriptor");
        auto line = query.get< std::string >("line");
        result.push_back(std::make_pair(fileDescriptor, line));
    }
    return result;
}

} // namespace libdnf
