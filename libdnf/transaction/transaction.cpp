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


#include "transaction.hpp"
#include "comps_environment.hpp"
#include "comps_group.hpp"
#include "rpm_package.hpp"
#include "transaction_item.hpp"

#include "libdnf/transaction/db/console_output.hpp"
#include "libdnf/transaction/db/trans.hpp"
#include "libdnf/transaction/db/trans_with.hpp"
#include "libdnf/transaction/db/rpm.hpp"
#include "libdnf/utils/bgettext/bgettext-lib.h"

#include <fmt/format.h>


namespace libdnf::transaction {

Transaction::Transaction(libdnf::utils::SQLite3 & conn, int64_t pk)
  : conn{conn}
{
    dbSelect(pk);
    runtime_packages = load_transaction_runtime_packages(*this);
    console_output = console_output_load(*this);
}

Transaction::Transaction(libdnf::utils::SQLite3 & conn)
  : conn{conn}
{
}

bool
Transaction::operator==(const Transaction &other) const
{
    return get_id() == other.get_id() && get_dt_begin() == other.get_dt_begin() &&
           get_rpmdb_version_begin() == other.get_rpmdb_version_begin();
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
    return get_id() > other.get_id() || get_dt_begin() > other.get_dt_begin() ||
           get_rpmdb_version_begin() > other.get_rpmdb_version_begin();
}

/**
 * \param other transaction to compare with
 * \return true if other transaction is newer
 */
bool
Transaction::operator>(const Transaction &other) const
{
    return get_id() < other.get_id() || get_dt_begin() < other.get_dt_begin() ||
           get_rpmdb_version_begin() < other.get_rpmdb_version_begin();
}

void
Transaction::dbSelect(int64_t pk)
{
    auto query = trans_select_new_query(conn);
    trans_select(*query, pk, *this);
}

/**
 * Loader for the transaction items.
 * \return list of transaction items associated with the transaction
 */
std::vector< TransactionItemPtr >
Transaction::getItems()
{
    if (!items.empty()) {
        return items;
    }
    std::vector< TransactionItemPtr > result;

    auto rpms = get_transaction_packages(*this);
    result.insert(result.end(), rpms.begin(), rpms.end());

    auto comps_groups = CompsGroup::getTransactionItems(*this);
    result.insert(result.end(), comps_groups.begin(), comps_groups.end());

    auto comps_environments = CompsEnvironment::getTransactionItems(*this);
    result.insert(result.end(), comps_environments.begin(), comps_environments.end());

    return result;
}


void
Transaction::begin()
{
    if (id != 0) {
        throw std::runtime_error(_("Transaction has already began!"));
    }
    dbInsert();
    save_transaction_runtime_packages(*this);
    saveItems();
}

void
Transaction::finish(TransactionState state)
{
    // save states to the database before checking for UNKNOWN state
    for (auto i : getItems()) {
        i->saveState();
    }

    for (auto i : getItems()) {
        if (i->get_state() == TransactionItemState::UNKNOWN) {
            throw std::runtime_error(
                fmt::format(_("TransactionItem state is not set: {}"), i->getItem()->toStr()));
        }
    }

    set_state(state);
    dbUpdate();
}

void
Transaction::dbInsert()
{
    auto query = trans_insert_new_query(conn);
    trans_insert(*query, *this);
}

void
Transaction::dbUpdate()
{
    auto query = trans_update_new_query(get_connection());
    trans_update(*query, *this);
}

TransactionItemPtr
Transaction::addItem(std::shared_ptr< Item > item,
                                           const std::string &repoid,
                                           TransactionItemAction action,
                                           TransactionItemReason reason)
{
    for (auto & i : items) {
        if (i->getItem()->toStr() != item->toStr()) {
            continue;
        }
        if (i->get_repoid() != repoid) {
            continue;
        }
        if (i->get_action() != action) {
            continue;
        }
        if (reason > i->get_reason()) {
            // use the more significant reason
            i->set_reason(reason);
        }
        // don't add duplicates to the list
        // return an existing transaction item if exists
        return i;
    }
    auto trans_item = std::make_shared< TransactionItem >(*this);
    trans_item->setItem(item);
    trans_item->set_repoid(repoid);
    trans_item->set_action(action);
    trans_item->set_reason(reason);
    items.push_back(trans_item);
    return trans_item;
}

void
Transaction::saveItems()
{
    // TODO: remove all existing items from the database first?
    for (auto i : items) {
        i->save();
    }

    /* this has to be done in a separate loop to make sure
     * that all the items already have ID assigned
     */
    for (auto i : items) {
        i->saveReplacedBy();
    }
}


/**
 * Save console output line for current transaction to the database. Transaction has
 *  to be saved in advance, otherwise an exception will be thrown.
 * \param fileDescriptor UNIX file descriptor index (1 = stdout, 2 = stderr).
 * \param line console output content
 */
void Transaction::add_console_output_line(int file_descriptor, const std::string & line) {
    if (!get_id()) {
        throw std::runtime_error(_("Can't add console output to unsaved transaction"));
    }

    // save the line to the database
    console_output_insert_line(*this, file_descriptor, line);

    // also store the line in the console_output vector
    console_output.emplace_back(file_descriptor, line);
}


}  // namespace libdnf::transaction
