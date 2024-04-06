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

#include "libdnf5-cli/output/transaction_table.hpp"

namespace libdnf5::cli::output {

const char * action_color(libdnf5::transaction::TransactionItemAction action) {
    switch (action) {
        case libdnf5::transaction::TransactionItemAction::INSTALL:
        case libdnf5::transaction::TransactionItemAction::UPGRADE:
        case libdnf5::transaction::TransactionItemAction::REINSTALL:
        case libdnf5::transaction::TransactionItemAction::REASON_CHANGE:
        case libdnf5::transaction::TransactionItemAction::ENABLE:
            return "green";
        case libdnf5::transaction::TransactionItemAction::DOWNGRADE:
        case libdnf5::transaction::TransactionItemAction::RESET:
            return "magenta";
        case libdnf5::transaction::TransactionItemAction::REMOVE:
        case libdnf5::transaction::TransactionItemAction::DISABLE:
            return "red";
        case libdnf5::transaction::TransactionItemAction::REPLACED:
            return "halfbright";
    }

    libdnf_throw_assertion("Unexpected action in print_transaction_table: {}", libdnf5::utils::to_underlying(action));
}

}  // namespace libdnf5::cli::output
