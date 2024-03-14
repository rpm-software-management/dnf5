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

#ifndef LIBDNF5_TRANSACTION_TRANSACTION_HPP
#define LIBDNF5_TRANSACTION_TRANSACTION_HPP

#include "comps_environment.hpp"
#include "comps_group.hpp"
#include "rpm_package.hpp"
#include "transaction_item.hpp"

#include "libdnf5/base/transaction_environment.hpp"
#include "libdnf5/base/transaction_group.hpp"
#include "libdnf5/base/transaction_module.hpp"
#include "libdnf5/base/transaction_package.hpp"
#include "libdnf5/defs.h"

#include <memory>
#include <set>
#include <string>


namespace libdnf5::transaction {

class TransactionDbUtils;
class CompsEnvironment;
class CompsEnvironmentDbUtils;

class Transaction;
using TransactionWeakPtr = libdnf5::WeakPtr<Transaction, false>;

}  // namespace libdnf5::transaction


namespace libdnf5::transaction {

class Item;
class Transformer;
class TransactionHistory;


// @replaces libdnf:transaction/Types.hpp:enum:TransactionState
enum class TransactionState : int { STARTED = 1, OK = 2, ERROR = 3 };

LIBDNF_API std::string transaction_state_to_string(TransactionState state);
LIBDNF_API TransactionState transaction_state_from_string(const std::string & state);

class LIBDNF_API InvalidTransactionState : public libdnf5::Error {
public:
    InvalidTransactionState(const std::string & state);

    const char * get_domain_name() const noexcept override { return "libdnf5::transaction"; }
    const char * get_name() const noexcept override { return "InvalidTransactionState"; }
};


/// Transaction holds information about a transaction.
/// It contains transaction items such as packages, comps groups and environments.
/// Transaction object are used to retrieve information about past transactions
/// from the transaction history database as well as for performing a transaction
/// to change packages on disk.
///
// @replaces libdnf:transaction/Transaction.hpp:class:Transaction
class LIBDNF_API Transaction {
public:
    using State = TransactionState;

    virtual ~Transaction();

    Transaction(const Transaction & src);
    Transaction & operator=(const Transaction & src);

    Transaction(Transaction && src) noexcept;
    Transaction & operator=(Transaction && src) noexcept;


    bool operator==(const Transaction & other) const;
    bool operator<(const Transaction & other) const;
    bool operator>(const Transaction & other) const;

    /// Get Transaction database id (primary key)
    /// Return 0 if the id wasn't set yet
    ///
    // @replaces libdnf:transaction/Transaction.hpp:method:Transaction.getId()
    int64_t get_id() const noexcept;

    /// Get date and time of the transaction start
    ///
    // @replaces libdnf:transaction/Transaction.hpp:method:Transaction.get_dt_begin()
    int64_t get_dt_start() const noexcept;

    /// Get date and time of the transaction end
    ///
    // @replaces libdnf:transaction/Transaction.hpp:method:Transaction.get_dt_begin()
    int64_t get_dt_end() const noexcept;

    /// Get RPM database version before the transaction
    /// Format: `<rpm_count>`:`<sha1 of sorted SHA1HEADER fields of installed RPMs>`
    ///
    // @replaces libdnf:transaction/Transaction.hpp:method:Transaction.get_rpmdb_version_begin()
    const std::string & get_rpmdb_version_begin() const noexcept;

    /// Get RPM database version after the transaction
    /// Format: `<rpm_count>`:`<sha1 of sorted SHA1HEADER fields of installed RPMs>`
    ///
    // @replaces libdnf:transaction/Transaction.hpp:method:Transaction.get_rpmdb_version_end()
    const std::string & get_rpmdb_version_end() const noexcept;

    /// Get $releasever variable value that was used during the transaction
    ///
    // @replaces libdnf:transaction/Transaction.hpp:method:Transaction.get_releasever()
    const std::string & get_releasever() const noexcept;

    /// Get UID of a user that started the transaction
    ///
    // @replaces libdnf:transaction/Transaction.hpp:method:Transaction.get_user_id()
    uint32_t get_user_id() const noexcept;

    /// Get the description of the transaction (e.g. the CLI command that was executed)
    ///
    // @replaces libdnf:transaction/Transaction.hpp:method:Transaction.get_cmdline()
    const std::string & get_description() const noexcept;

    /// Get a user-specified comment describing the transaction
    const std::string & get_comment() const noexcept;

    /// Get transaction state
    ///
    // @replaces libdnf:transaction/Transaction.hpp:method:Transaction.getState()
    State get_state() const noexcept;

    /// Return all comps environments associated with the transaction
    ///
    // @replaces libdnf:transaction/Transaction.hpp:method:Transaction.getItems()
    std::vector<CompsEnvironment> & get_comps_environments();

    /// Return all comps groups associated with the transaction
    ///
    // @replaces libdnf:transaction/Transaction.hpp:method:Transaction.getItems()
    std::vector<CompsGroup> & get_comps_groups();

    /// Return all rpm packages associated with the transaction
    ///
    // @replaces libdnf:transaction/Transaction.hpp:method:Transaction.getItems()
    std::vector<Package> & get_packages();

    /// @warning This method is experimental/unstable and should not be relied on. It may be removed without warning
    /// Serialize the transaction into a json data format which can be later loaded
    /// into a `libdnf5::Goal` and replayed.
    std::string serialize();

private:
    friend Transformer;
    friend libdnf5::base::Transaction;
    friend TransactionDbUtils;
    friend TransactionHistory;
    friend CompsEnvironment;
    friend CompsEnvironmentDbUtils;

    /// Constructs a new, empty `Transaction` object. The object is expected to
    /// be filled by the user and saved to the database.
    ///
    /// @param base The base.
    LIBDNF_LOCAL explicit Transaction(const libdnf5::BaseWeakPtr & base);

    /// Constructs the transaction with a known id which needs to exist in the
    /// database. The data are then lazily loaded from the database on first call
    /// of an attribute getter.
    ///
    /// @param base The base.
    /// @param id The id of the transaction.
    LIBDNF_LOCAL Transaction(const libdnf5::BaseWeakPtr & base, int64_t id);

    /// Set Transaction database id (primary key)
    ///
    // @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setId(int64_t value)
    LIBDNF_LOCAL void set_id(int64_t value);

    /// Set a user-specified comment describing the transaction
    LIBDNF_LOCAL void set_comment(const std::string & value);

    /// Set date and time of the transaction start
    ///
    // @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setDtBegin(int64_t value)
    LIBDNF_LOCAL void set_dt_start(int64_t value);

    /// Set date and time of the transaction end
    ///
    // @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setDtEnd(int64_t value)
    LIBDNF_LOCAL void set_dt_end(int64_t value);

    /// Set the description of the transaction (e.g. the CLI command that was executed)
    ///
    // @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setCmdline(const std::string & value)
    LIBDNF_LOCAL void set_description(const std::string & value);

    /// Set UID of a user that started the transaction
    ///
    // @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setUserId(uint32_t value)
    LIBDNF_LOCAL void set_user_id(uint32_t value);

    /// Set $releasever variable value that was used during the transaction
    ///
    // @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setReleasever(const std::string & value)
    LIBDNF_LOCAL void set_releasever(const std::string & value);

    /// Set RPM database version after the transaction
    /// Format: `<rpm_count>`:`<sha1 of sorted SHA1HEADER fields of installed RPMs>`
    ///
    // @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setRpmdbVersionEnd(const std::string & value)
    LIBDNF_LOCAL void set_rpmdb_version_end(const std::string & value);

    /// Set RPM database version before the transaction
    /// Format: `<rpm_count>`:`<sha1 of sorted SHA1HEADER fields of installed RPMs>`
    ///
    // @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setRpmdbVersionBegin(const std::string & value)
    LIBDNF_LOCAL void set_rpmdb_version_begin(const std::string & value);

    /// Set transaction state
    ///
    // @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.setState(libdnf::TransactionState value)
    LIBDNF_LOCAL void set_state(State value);

    /// Create a new rpm package in the transaction and return a reference to it.
    /// The package is owned by the transaction.
    ///
    // @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.addItem(std::shared_ptr<Item> item, const std::string & repoid, libdnf::TransactionItemAction action, libdnf::TransactionItemReason reason)
    LIBDNF_LOCAL Package & new_package();

    /// Fill the transaction packages.
    LIBDNF_LOCAL void fill_transaction_packages(
        const std::vector<libdnf5::base::TransactionPackage> & transaction_packages);

    /// Fill the transaction groups.
    /// @param transaction_groups Groups that are part of the transaction
    /// @param installed_names Names of currently installed plus inbound packages
    LIBDNF_LOCAL void fill_transaction_groups(
        const std::vector<libdnf5::base::TransactionGroup> & transaction_groups,
        const std::set<std::string> & installed_names);

    /// Fill the transaction environmental groups.
    /// @param transaction_groups Environmental groups that are part of the transaction
    /// @param installed_names Ids of currently installed plus inbound groups
    LIBDNF_LOCAL void fill_transaction_environments(
        const std::vector<libdnf5::base::TransactionEnvironment> & transaction_environments,
        const std::set<std::string> & installed_group_ids);

    /// Create a new comps group in the transaction and return a reference to it.
    /// The group is owned by the transaction.
    ///
    // @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.addItem(std::shared_ptr<Item> item, const std::string & repoid, libdnf::TransactionItemAction action, libdnf::TransactionItemReason reason)
    LIBDNF_LOCAL CompsGroup & new_comps_group();

    /// Create a new comps environment in the transaction and return a reference to it.
    /// The environment is owned by the transaction.
    ///
    // @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.addItem(std::shared_ptr<Item> item, const std::string & repoid, libdnf::TransactionItemAction action, libdnf::TransactionItemReason reason)
    LIBDNF_LOCAL CompsEnvironment & new_comps_environment();

    /// Start the transaction by inserting it into the database
    ///
    // @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.begin()
    LIBDNF_LOCAL void start();

    /// Finish the transaction by updating it's state in the database
    ///
    // @replaces libdnf:transaction/private/Transaction.hpp:method:Transaction.finish(libdnf::TransactionState state)
    LIBDNF_LOCAL void finish(TransactionState state);

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_TRANSACTION_HPP
