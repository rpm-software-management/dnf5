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

#ifndef LIBDNF_CLI_EXCEPTION_HPP
#define LIBDNF_CLI_EXCEPTION_HPP

#include "exit-codes.hpp"

#include "libdnf/base/transaction.hpp"
#include "libdnf/common/exception.hpp"

namespace libdnf::cli {

/// Base class for libdnf-cli exceptions.
class Error : public libdnf::Error {
public:
    using libdnf::Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf::cli"; }
    const char * get_name() const noexcept override { return "Error"; }
};


/// Exception is thrown when the user does not confirm the transaction.
class AbortedByUserError : public Error {
public:
    AbortedByUserError();
    const char * get_name() const noexcept override { return "AbortedByUserError"; }
};


/// Exception is thrown when libdnf fails to resolve the transaction.
class GoalResolveError : public Error {
public:
    /// Construct Error from a list of string representations of resolve logs.
    /// @param resolve_logs List of resolve logs
    explicit GoalResolveError(const std::vector<std::string> resolve_logs);

    /// A constructor that extracts errors from the transaction.
    ///
    /// @param transaction The failed transaction
    explicit GoalResolveError(const libdnf::base::Transaction & transaction)
        : GoalResolveError(transaction.get_resolve_logs_as_strings()) {}

    const char * get_name() const noexcept override { return "GoalResolveError"; }
    const char * what() const noexcept override;

private:
    std::vector<std::string> resolve_logs;
};


/// Exception used when non-standard exit code should be returned
class CommandExitError : public Error {
public:
    /// Constructs the error with given exit code and with custom formatted error
    /// message.
    ///
    /// @param exit_code The exit code
    /// @format The format string for the message
    /// @args The format arguments
    template <typename... Ss>
    explicit CommandExitError(int exit_code, BgettextMessage format, Ss &&... args)
        : Error(format, std::forward<Ss>(args)...),
          exit_code(exit_code) {}

    const char * get_name() const noexcept override { return "CommandExitError"; }

    /// @return The exit code
    int get_exit_code() const noexcept { return exit_code; }

private:
    int exit_code;
};

}  // namespace libdnf::cli

#endif
