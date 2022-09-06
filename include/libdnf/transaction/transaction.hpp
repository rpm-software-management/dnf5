/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
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

#include "comps_environment.hpp"
#include "comps_group.hpp"
#include "rpm_package.hpp"
#include "transaction_item.hpp"

#include "libdnf/base/transaction_package.hpp"

#include <memory>
#include <set>
#include <string>


namespace libdnf::transaction {

class Transaction;
using TransactionWeakPtr = libdnf::WeakPtr<Transaction, false>;

}  // namespace libdnf::transaction


namespace libdnf::transaction {

class Item;
class Transformer;


/// @replaces libdnf:transaction/Types.hpp:enum:TransactionState
enum class TransactionState : int { STARTED = 1, OK = 2, ERROR = 3 };

std::string transaction_state_to_string(TransactionState state);
TransactionState transaction_state_from_string(const std::string & state);

class InvalidTransactionState : public libdnf::Error {
public:
    InvalidTransactionState(const std::string & state);

    const char * get_domain_name() const noexcept override { return "libdnf::transaction"; }
    const char * get_name() const noexcept override { return "InvalidTransactionState"; }
};


/// Transaction holds information about a transaction.
/// It contains transaction items such as packages, comps groups and environments.
/// Transaction object are used to retrieve information about past transactions
/// from the transaction history database as well as for performing a transaction
/// to change packages on disk.
///
/// @replaces libdnf:transaction/Transaction.hpp:class:Transaction
class Transaction {
public:
    using State = TransactionState;

    /// Constructs a new, empty `Transaction` object. The object is expected to
    /// be filled by the user and saved to the database.
    ///
    /// @param base The base.
    explicit Transaction(const libdnf::BaseWeakPtr & base);

    /// Constructs the transaction with a known id which needs to exist in the
    /// database. The data are then lazily loaded from the database on first call
    /// of an attribute getter.
    ///
    /// @param base The base.
    /// @param id The id of the transaction.
    Transaction(const libdnf::BaseWeakPtr & base, int64_t id);

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
    int64_t get_dt_start() const noexcept { return dt_begin; }

    /// Set date and time of the transaction start
    ///
    /// @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setDtBegin(int64_t value)
    void set_dt_start(int64_t value) { dt_begin = value; }

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

    /// Get the description of the transaction (e.g. the CLI command that was executed)
    ///
    /// @replaces libdnf:transaction/Transaction.hpp:method:Transaction.get_cmdline()
    const std::string & get_description() const noexcept { return description; }

    /// Set the description of the transaction (e.g. the CLI command that was executed)
    ///
    /// @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setCmdline(const std::string & value)
    void set_description(const std::string & value) { description = value; }

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

    /// Get lines recorded during the transaction
    ///
    /// @replaces libdnf:transaction/Transaction.hpp:method:Transaction.getConsoleOutput()
    const std::vector<std::pair<int, std::string>> & get_console_output();

    /// Record a line printed during the transction to the database and also store it in the object
    ///
    /// The method must be called only on a started transaction
    void add_console_output_line(int file_descriptor, const std::string & line);

    /// Return all comps environments associated with the transaction
    ///
    /// @replaces libdnf:transaction/Transaction.hpp:method:Transaction.getItems()
    std::vector<CompsEnvironment> & get_comps_environments();

    /// Create a new comps environment in the transaction and return a reference to it.
    /// The environment is owned by the transaction.
    ///
    /// @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.addItem(std::shared_ptr<Item> item, const std::string & repoid, libdnf::TransactionItemAction action, libdnf::TransactionItemReason reason)
    CompsEnvironment & new_comps_environment();

    /// Return all comps groups associated with the transaction
    ///
    /// @replaces libdnf:transaction/Transaction.hpp:method:Transaction.getItems()
    std::vector<CompsGroup> & get_comps_groups();

    /// Create a new comps group in the transaction and return a reference to it.
    /// The group is owned by the transaction.
    ///
    /// @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.addItem(std::shared_ptr<Item> item, const std::string & repoid, libdnf::TransactionItemAction action, libdnf::TransactionItemReason reason)
    CompsGroup & new_comps_group();

    /// Return all rpm packages associated with the transaction
    ///
    /// @replaces libdnf:transaction/Transaction.hpp:method:Transaction.getItems()
    std::vector<Package> & get_packages();

    /// Create a new rpm package in the transaction and return a reference to it.
    /// The package is owned by the transaction.
    ///
    /// @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.addItem(std::shared_ptr<Item> item, const std::string & repoid, libdnf::TransactionItemAction action, libdnf::TransactionItemReason reason)
    Package & new_package();

    /// Fill the transaction packages.
    void fill_transaction_packages(const std::vector<libdnf::base::TransactionPackage> & transaction_packages);

    /// Start the transaction by inserting it into the database
    ///
    /// @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.begin()
    /// TODO(lukash) make the whole transaction creation API private
    void start();

    /// Finish the transaction by updating it's state in the database
    ///
    /// @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.finish(libdnf::TransactionState state)
    void finish(TransactionState state);

protected:
    friend Transformer;

private:
    int64_t id{0};

    int64_t dt_begin = 0;
    int64_t dt_end = 0;
    std::string rpmdb_version_begin;
    std::string rpmdb_version_end;
    // TODO(dmach): move to a new "vars" table?
    std::string releasever;
    uint32_t user_id = 0;
    std::string description;
    std::string comment;
    State state = State::STARTED;

    std::optional<std::vector<std::pair<int, std::string>>> console_output;

    std::optional<std::vector<CompsEnvironment>> comps_environments;
    std::optional<std::vector<CompsGroup>> comps_groups;
    std::optional<std::vector<Package>> packages;

    BaseWeakPtr base;
};

}  // namespace libdnf::transaction

#endif  // LIBDNF_TRANSACTION_TRANSACTION_HPP
