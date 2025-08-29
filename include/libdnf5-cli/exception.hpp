// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_CLI_EXCEPTION_HPP
#define LIBDNF5_CLI_EXCEPTION_HPP

#include "exit-codes.hpp"

#include "libdnf5-cli/defs.h"

#include <libdnf5/base/transaction.hpp>
#include <libdnf5/common/exception.hpp>

namespace libdnf5::cli {

/// Base class for libdnf5-cli exceptions.
class Error : public libdnf5::Error {
public:
    using libdnf5::Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::cli"; }
    const char * get_name() const noexcept override { return "Error"; }
};


/// Exception is thrown when the user does not confirm the transaction.
class LIBDNF_CLI_API AbortedByUserError : public Error {
public:
    AbortedByUserError();
    const char * get_name() const noexcept override { return "AbortedByUserError"; }
};


/// Exception is thrown when the user does not have enough privileges to perform requested operation.
class InsufficientPrivilegesError : public Error {
public:
    using Error::Error;
    const char * get_name() const noexcept override { return "InsufficientPrivilegesError"; }
};


/// Exception is thrown when trying to write to a read-only system
class ReadOnlySystemError : public Error {
public:
    using Error::Error;
    const char * get_name() const noexcept override { return "ReadOnlySystemError"; }
};


/// Exception is thrown when libdnf5 fails to resolve the transaction.
class LIBDNF_CLI_API GoalResolveError : public Error {
public:
    /// Construct Error from a list of string representations of resolve logs.
    /// @param resolve_logs List of resolve logs
    explicit GoalResolveError(const std::vector<std::string> resolve_logs);

    /// A constructor that extracts errors from the transaction.
    ///
    /// @param transaction The failed transaction
    explicit GoalResolveError(const libdnf5::base::Transaction & transaction)
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
    /// @param format The format string for the message
    /// @param args The format arguments
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

/// Exception used when non-standard exit code should be returned without displaying any message to the user
class LIBDNF_CLI_API SilentCommandExitError : public Error {
public:
    /// Constructs the error with given exit code.
    ///
    /// @param exit_code The exit code
    explicit SilentCommandExitError(int exit_code);

    const char * get_name() const noexcept override { return "SilentCommandExitError"; }

    /// @return The exit code
    int get_exit_code() const noexcept { return exit_code; }

private:
    int exit_code;
};

}  // namespace libdnf5::cli

#endif
