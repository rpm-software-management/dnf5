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


#ifndef LIBDNF_TRANSACTION_TRANSACTION_HPP
#define LIBDNF_TRANSACTION_TRANSACTION_HPP


#include "libdnf/utils/sqlite3/sqlite3.hpp"

#include <memory>
#include <set>
#include <string>

namespace libdnf::transaction {
class Transaction;
typedef std::shared_ptr<Transaction> TransactionPtr;
}  // namespace libdnf::transaction

#include "Item.hpp"
#include "transaction_item.hpp"


namespace libdnf::transaction {


class Item;
class Transformer;


/// @replaces libdnf:transaction/Types.hpp:enum:TransactionState
enum class TransactionState : int { UNKNOWN = 0, DONE = 1, ERROR = 2 };


/// @replaces libdnf:transaction/Transaction.hpp:class:Transaction
class Transaction {
public:
    using State = TransactionState;

    explicit Transaction() = default;
    // load from db
    explicit Transaction(libdnf::utils::SQLite3 & conn);
    explicit Transaction(libdnf::utils::SQLite3 & conn, int64_t pk);
    virtual ~Transaction() = default;

    bool operator==(const Transaction & other) const;
    bool operator<(const Transaction & other) const;
    bool operator>(const Transaction & other) const;

    /// Get Transaction database id (primary key)
    /// Return 0 if the id wasn't set yet
    ///
    /// @replaces libdnf:transaction/Transaction.hpp:method:Transaction.getId()
    int64_t get_id() const noexcept { return id; }

    /// Set Transaction database id (primary key)
    ///
    /// @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setId(int64_t value)
    void set_id(int64_t value) { id = value; }

    /// Get date and time of the transaction start
    ///
    /// @replaces libdnf:transaction/Transaction.hpp:method:Transaction.get_dt_begin()
    int64_t get_dt_begin() const noexcept { return dt_begin; }

    /// Set date and time of the transaction start
    ///
    /// @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setDtBegin(int64_t value)
    void set_dt_begin(int64_t value) { dt_begin = value; }

    /// Get date and time of the transaction end
    ///
    /// @replaces libdnf:transaction/Transaction.hpp:method:Transaction.get_dt_begin()
    int64_t get_dt_end() const noexcept { return dt_end; }

    /// Set date and time of the transaction end
    ///
    /// @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setDtEnd(int64_t value)
    void set_dt_end(int64_t value) { dt_end = value; }

    /// Get RPM database version before the transaction
    /// Format: <rpm_count>:<sha1 of sorted SHA1HEADER fields of installed RPMs>
    ///
    /// @replaces libdnf:transaction/Transaction.hpp:method:Transaction.get_rpmdb_version_begin()
    const std::string & get_rpmdb_version_begin() const noexcept { return rpmdb_version_begin; }

    /// Set RPM database version before the transaction
    /// Format: <rpm_count>:<sha1 of sorted SHA1HEADER fields of installed RPMs>
    ///
    /// @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setRpmdbVersionBegin(const std::string & value)
    void set_rpmdb_version_begin(const std::string & value) { rpmdb_version_begin = value; }

    /// Get RPM database version after the transaction
    /// Format: <rpm_count>:<sha1 of sorted SHA1HEADER fields of installed RPMs>
    ///
    /// @replaces libdnf:transaction/Transaction.hpp:method:Transaction.get_rpmdb_version_end()
    const std::string & get_rpmdb_version_end() const noexcept { return rpmdb_version_end; }

    /// Set RPM database version after the transaction
    /// Format: <rpm_count>:<sha1 of sorted SHA1HEADER fields of installed RPMs>
    ///
    /// @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setRpmdbVersionEnd(const std::string & value)
    void set_rpmdb_version_end(const std::string & value) { rpmdb_version_end = value; }

    /// Get $releasever variable value that was used during the transaction
    ///
    /// @replaces libdnf:transaction/Transaction.hpp:method:Transaction.get_releasever()
    const std::string & get_releasever() const noexcept { return releasever; }

    /// Set $releasever variable value that was used during the transaction
    ///
    /// @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setReleasever(const std::string & value)
    void set_releasever(const std::string & value) { releasever = value; }

    /// Get UID of a user that started the transaction
    ///
    /// @replaces libdnf:transaction/Transaction.hpp:method:Transaction.get_user_id()
    uint32_t get_user_id() const noexcept { return user_id; }

    /// Set UID of a user that started the transaction
    ///
    /// @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setUserId(uint32_t value)
    void set_user_id(uint32_t value) { user_id = value; }

    /// Get command line of a program that started the transaction
    ///
    /// @replaces libdnf:transaction/Transaction.hpp:method:Transaction.get_cmdline()
    const std::string & get_cmdline() const noexcept { return cmdline; }

    /// Set command line of a program that started the transaction
    ///
    /// @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setCmdline(const std::string & value)
    void set_cmdline(const std::string & value) { cmdline = value; }

    /// Get a user-specified comment describing the transaction
    const std::string & get_comment() const noexcept { return comment; }

    /// Set a user-specified comment describing the transaction
    void set_comment(const std::string & value) { comment = value; }

    /// Get transaction state
    ///
    /// @replaces libdnf:transaction/Transaction.hpp:method:Transaction.getState()
    State get_state() const noexcept { return state; }

    /// Set transaction state
    ///
    /// @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setState(libdnf::TransactionState value)
    void set_state(State value) { state = value; }

    virtual std::vector<TransactionItemPtr> getItems();
    const std::set<std::shared_ptr<RPMItem>> getSoftwarePerformedWith() const;
    std::vector<std::pair<int, std::string>> getConsoleOutput() const;

    void begin();
    void finish(TransactionState state);
    TransactionItemPtr addItem(
        std::shared_ptr<Item> item,
        const std::string & repoid,
        TransactionItemAction action,
        TransactionItemReason reason);

    void addConsoleOutputLine(int fileDescriptor, const std::string & line);
    void addSoftwarePerformedWith(std::shared_ptr<RPMItem> software);

    libdnf::utils::SQLite3 & get_connection() { return conn; }

protected:
    friend Item;
    friend Transformer;

    void dbSelect(int64_t transaction_id);

    void saveItems();
    std::vector<TransactionItemPtr> items;

    void dbInsert();
    void dbUpdate();

    std::set<std::shared_ptr<RPMItem>> softwarePerformedWith;

    friend class TransactionItem;
    libdnf::utils::SQLite3 & conn;

private:
    int64_t id = 0;
    int64_t dt_begin = 0;
    int64_t dt_end = 0;
    std::string rpmdb_version_begin;
    std::string rpmdb_version_end;
    // TODO(dmach): move to a new "vars" table?
    std::string releasever;
    uint32_t user_id = 0;
    std::string cmdline;
    // TODO(dmach): backport comment support from dnf-4-master
    std::string comment;
    State state = State::UNKNOWN;
};


}  // namespace libdnf::transaction


#endif  // LIBDNF_TRANSACTION_TRANSACTION_HPP
