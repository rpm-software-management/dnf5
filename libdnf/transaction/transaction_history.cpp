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


#include "libdnf/transaction/transaction_history.hpp"

#include "db/trans.hpp"

#include "libdnf/base/base.hpp"


namespace libdnf::transaction {


TransactionHistory::TransactionHistory(const libdnf::BaseWeakPtr & base) : base{base} {}


TransactionHistory::TransactionHistory(libdnf::Base & base) : TransactionHistory(base.get_weak_ptr()) {}


Transaction TransactionHistory::new_transaction() {
    return Transaction(base);
}

std::vector<int64_t> TransactionHistory::list_transaction_ids() {
    return TransactionDbUtils::select_transaction_ids(base);
}

std::vector<Transaction> TransactionHistory::list_transactions(const std::vector<int64_t> & ids) {
    return TransactionDbUtils::select_transactions_by_ids(base, ids);
}

std::vector<Transaction> TransactionHistory::list_transactions(int64_t start, int64_t end) {
    return TransactionDbUtils::select_transactions_by_range(base, start, end);
}

std::vector<Transaction> TransactionHistory::list_all_transactions() {
    return TransactionDbUtils::select_transactions_by_ids(base, {});
}

BaseWeakPtr TransactionHistory::get_base() const {
    return base;
}

}  // namespace libdnf::transaction
