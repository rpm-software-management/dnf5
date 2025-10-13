// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_BASE_ACTIVE_TRANSACTION_INFO_HPP
#define LIBDNF5_BASE_ACTIVE_TRANSACTION_INFO_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"

#include <unistd.h>

#include <ctime>
#include <memory>
#include <string>

namespace libdnf5::base {

class LIBDNF_API ActiveTransactionInfoParseError : public libdnf5::Error {
public:
    using libdnf5::Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::base"; }
    const char * get_name() const noexcept override { return "ActiveTransactionInfoParseError"; }
};

class LIBDNF_API ActiveTransactionInfo {
public:
    ActiveTransactionInfo();
    ActiveTransactionInfo(const ActiveTransactionInfo &);
    ActiveTransactionInfo(ActiveTransactionInfo &&) noexcept;
    ActiveTransactionInfo & operator=(const ActiveTransactionInfo &);
    ActiveTransactionInfo & operator=(ActiveTransactionInfo &&) noexcept;
    ~ActiveTransactionInfo();

    // Getters
    /// @brief Get the transaction description
    /// @return Description string
    const std::string & get_description() const noexcept;

    /// @brief Get the transaction comment
    /// @return Comment string
    const std::string & get_comment() const noexcept;

    /// @brief Get the process ID of the transaction
    /// @return Process ID, or -1 if not available
    pid_t get_pid() const noexcept;

    /// @brief Get the transaction start time as Unix timestamp
    /// @return Unix timestamp (seconds since epoch), or 0 if not available
    time_t get_start_time() const noexcept;

    // Setters
    void set_description(const std::string & description);
    void set_comment(const std::string & comment);
    void set_pid(pid_t pid);
    void set_start_time(time_t start_time);

    // TOML conversion methods
    /// @brief Convert ActiveTransactionInfo to TOML string format
    /// @return TOML formatted string representation
    /// @throws std::runtime_error if TOML formatting fails
    std::string to_toml() const;

    /// @brief Create ActiveTransactionInfo from TOML string
    /// @param toml_string The TOML string to parse
    /// @return ActiveTransactionInfo object parsed from TOML
    /// @throws ActiveTransactionInfoParseError if TOML parsing fails
    static ActiveTransactionInfo from_toml(const std::string & toml_string);

private:
    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_ACTIVE_TRANSACTION_INFO_HPP
