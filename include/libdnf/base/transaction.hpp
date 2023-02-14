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


#ifndef LIBDNF_BASE_TRANSACTION_HPP
#define LIBDNF_BASE_TRANSACTION_HPP

#include "libdnf/base/base_weak.hpp"
#include "libdnf/base/goal_elements.hpp"
#include "libdnf/base/log_event.hpp"
#include "libdnf/base/solver_problems.hpp"
#include "libdnf/common/proc.hpp"
#include "libdnf/rpm/transaction_callbacks.hpp"

#include <optional>


namespace libdnf::base {

class TransactionGroup;
class TransactionPackage;

/// Error related to processing transaction
class TransactionError : public Error {
public:
    using Error::Error;
    /// @return Error class' domain name"
    const char * get_domain_name() const noexcept override { return "libdnf::base"; }
    /// @return Error class' name"
    const char * get_name() const noexcept override { return "TransactionError"; }
};


class Transaction {
public:
    /// enum representing Transaction run result
    enum class TransactionRunResult {
        SUCCESS,
        ERROR_RERUN,
        ERROR_RESOLVE,
        ERROR_LOCK,
        ERROR_CHECK,
        ERROR_RPM_RUN,
    };

    Transaction(const Transaction & transaction);
    Transaction(Transaction && transaction);
    ~Transaction();

    /// Return basic overview about result of resolving transaction.
    /// To get complete information, use get_resolve_logs().
    libdnf::GoalProblem get_problems();

    /// Returns information about resolvement of Goal.
    /// @return A vector of LogEvent instances.
    const std::vector<libdnf::base::LogEvent> & get_resolve_logs() const;

    /// Returns information about resolvement of Goal as a list of printable messages
    /// @return A vector of string representations of problems.
    std::vector<std::string> get_resolve_logs_as_strings() const;

    /// @return the transaction packages.
    // TODO(jrohel): Return reference instead of copy?
    std::vector<libdnf::base::TransactionPackage> get_transaction_packages() const;

    /// @return the transaction groups.
    std::vector<libdnf::base::TransactionGroup> & get_transaction_groups() const;

    /// Check the transaction by running it with RPMTRANS_FLAG_TEST. The import
    /// of any necessary public keys will be requested, and transaction checks
    /// will be performed, but no changes to the installed package set will be
    /// made. These checks are performed automatically by run(); it is
    /// redundant to call test() before calling run().
    /// @return An enum describing the result of the transaction
    TransactionRunResult test();

    /// @brief Prepare, check and run the transaction.
    ///
    /// All the transaction metadata that was set (`description`, `user_id` or `comment`)
    /// is stored in the history database.
    ///
    /// To watch progress or trigger actions during specific transactions events,
    /// setup the `callbacks` object.
    ///
    /// @return An enum describing the result of running the transaction.
    TransactionRunResult run();

    /// @brief Setup callbacks to be called during rpm transaction.
    /// @param callbacks Implemented callbacks object.
    void set_callbacks(std::unique_ptr<libdnf::rpm::TransactionCallbacks> && callbacks);

    /// @brief Setup a description of the transaction.
    /// @param description Value could be the console command for CLI or verbose description for API usage.
    void set_description(const std::string & description);

    /// @brief Setup the id of the user that started the transaction. If not set, current login user UID is used.
    /// @param user_id UID value.
    void set_user_id(const uint32_t user_id);

    /// @brief Setup a comment to store in the history database along with the transaction.
    /// @param comment Any string value.
    void set_comment(const std::string & comment);

    /// Return string representation of the TransactionRunResult enum
    static std::string transaction_result_to_string(const TransactionRunResult result);

    /// Retrieve list of problems that occurred during transaction run attempt
    std::vector<std::string> get_transaction_problems() const noexcept;

private:
    friend class TransactionGroup;
    friend class TransactionPackage;
    friend class libdnf::Goal;

    Transaction(const libdnf::BaseWeakPtr & base);

    class Impl;
    std::unique_ptr<Impl> p_impl;

    std::unique_ptr<libdnf::rpm::TransactionCallbacks> callbacks;
    std::optional<uint32_t> user_id;
    std::string comment;
    std::string description;
};

}  // namespace libdnf::base

#endif  // LIBDNF_BASE_TRANSACTION_HPP
