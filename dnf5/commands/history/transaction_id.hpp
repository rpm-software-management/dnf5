// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


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
