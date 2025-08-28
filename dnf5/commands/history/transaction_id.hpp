// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_COMMANDS_HISTORY_TRANSACTION_ID_HPP
#define DNF5_COMMANDS_HISTORY_TRANSACTION_ID_HPP

#include <libdnf5/common/exception.hpp>
#include <libdnf5/transaction/transaction_history.hpp>

#include <cstdint>
#include <string>
#include <utility>


namespace dnf5 {


class InvalidIdRangeError : public libdnf5::Error {
public:
    InvalidIdRangeError(const std::string & id_range);

    const char * get_domain_name() const noexcept override { return "dnf5"; }
    const char * get_name() const noexcept override { return "InvalidIdRangeError"; }
};


std::vector<libdnf5::transaction::Transaction> list_transactions_from_specs(
    libdnf5::transaction::TransactionHistory & ts_history, const std::vector<std::string> & specs);


}  // namespace dnf5


#endif  // DNF5_COMMANDS_HISTORY_TRANSACTION_ID_HPP
