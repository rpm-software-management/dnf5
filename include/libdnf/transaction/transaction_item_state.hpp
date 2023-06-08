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

#ifndef LIBDNF_TRANSACTION_TRANSACTION_ITEM_STATE_HPP
#define LIBDNF_TRANSACTION_TRANSACTION_ITEM_STATE_HPP

#include "libdnf/common/exception.hpp"

#include <string>


namespace libdnf5::transaction {

enum class TransactionItemState : int { STARTED = 1, OK = 2, ERROR = 3 };


class InvalidTransactionItemState : public libdnf5::Error {
public:
    InvalidTransactionItemState(const std::string & state);

    const char * get_domain_name() const noexcept override { return "libdnf::transaction"; }
    const char * get_name() const noexcept override { return "InvalidTransactionItemState"; }
};


std::string transaction_item_state_to_string(TransactionItemState state);
TransactionItemState transaction_item_state_from_string(const std::string & state);

}  // namespace libdnf5::transaction

#endif  // LIBDNF_TRANSACTION_TRANSACTION_ITEM_STATE_HPP
