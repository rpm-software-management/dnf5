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
#include "libdnf/base/transaction_group.hpp"
#include "libdnf/base/transaction_package.hpp"
#include "libdnf/rpm/transaction_callbacks.hpp"

#include <optional>


namespace libdnf::base {

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

    /// Return basic overvie about result of resolving transaction. The get complete informaion use get_resolve_logs()
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

    /// Prepare, check and run the transaction. All the transaction metadata
    /// (`description`, `user_id` and `comment`) are stored in the history database.
    ///
    /// @param callbacks Callbacks to be called during rpm transaction.
    /// @param description Description of the transaction (the console command for CLI,
    //                     verbose description for API usage)
    /// @param user_id UID of the user that started the transaction.
    /// @param comment Any comment to store in the history database along with the transaction.
    /// @return An enum describing the result of running the transaction.
    TransactionRunResult run(
        std::unique_ptr<libdnf::rpm::TransactionCallbacks> && callbacks,
        const std::string & description,
        const std::optional<uint32_t> user_id,
        const std::optional<std::string> comment);

    /// Return string representation of the TransactionRunResult enum
    static std::string transaction_result_to_string(const TransactionRunResult result);

    /// Retrieve list of problems that occurred during transaction run attempt
    std::vector<std::string> get_transaction_problems() const noexcept;

private:
    friend class libdnf::Goal;

    Transaction(const libdnf::BaseWeakPtr & base);

    class Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf::base

#endif  // LIBDNF_BASE_TRANSACTION_HPP
