/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "transaction_callbacks_simple.hpp"

#include <libdnf5/rpm/nevra.hpp>
#include <libdnf5/rpm/transaction_callbacks.hpp>
#include <libdnf5/transaction/transaction.hpp>

#include <iostream>

namespace dnf5 {

void TransactionCallbacksSimple::transaction_start([[maybe_unused]] uint64_t total) {
    output_stream << "  Prepare transaction" << std::endl;
}


void TransactionCallbacksSimple::install_start(
    const libdnf5::rpm::TransactionItem & item, [[maybe_unused]] uint64_t total) {
    switch (item.get_action()) {
        case libdnf5::transaction::TransactionItemAction::UPGRADE:
            output_stream << "  Upgrading ";
            break;
        case libdnf5::transaction::TransactionItemAction::DOWNGRADE:
            output_stream << "  Downgrading ";
            break;
        case libdnf5::transaction::TransactionItemAction::REINSTALL:
            output_stream << "  Reinstalling ";
            break;
        case libdnf5::transaction::TransactionItemAction::INSTALL:
            output_stream << "  Installing ";
            break;
        case libdnf5::transaction::TransactionItemAction::REMOVE:
        case libdnf5::transaction::TransactionItemAction::REPLACED:
            break;
        case libdnf5::transaction::TransactionItemAction::REASON_CHANGE:
        case libdnf5::transaction::TransactionItemAction::ENABLE:
        case libdnf5::transaction::TransactionItemAction::DISABLE:
        case libdnf5::transaction::TransactionItemAction::RESET:
            throw std::logic_error(fmt::format(
                "Unexpected action in TransactionPackage: {}",
                static_cast<std::underlying_type_t<libdnf5::base::Transaction::TransactionRunResult>>(
                    item.get_action())));
    }
    output_stream << item.get_package().get_full_nevra() << std::endl;
}

void TransactionCallbacksSimple::uninstall_start(
    const libdnf5::rpm::TransactionItem & item, [[maybe_unused]] uint64_t total) {
    if (item.get_action() == libdnf5::transaction::TransactionItemAction::REMOVE) {
        output_stream << "  Erasing ";
    } else {
        output_stream << "  Cleanup ";
    }
    output_stream << item.get_package().get_full_nevra() << std::endl;
}

void TransactionCallbacksSimple::unpack_error(const libdnf5::rpm::TransactionItem & item) {
    output_stream << "  Unpack error: " << item.get_package().get_full_nevra() << std::endl;
}

void TransactionCallbacksSimple::cpio_error(const libdnf5::rpm::TransactionItem & item) {
    output_stream << "  Cpio error: " << item.get_package().get_full_nevra() << std::endl;
}

void TransactionCallbacksSimple::script_error(
    [[maybe_unused]] const libdnf5::rpm::TransactionItem * item,
    libdnf5::rpm::Nevra nevra,
    libdnf5::rpm::TransactionCallbacks::ScriptType type,
    uint64_t return_code) {
    output_stream << "  Error in " << script_type_to_string(type) << " scriptlet: " << to_full_nevra_string(nevra)
                  << " return code " << return_code << std::endl;
}

void TransactionCallbacksSimple::script_start(
    [[maybe_unused]] const libdnf5::rpm::TransactionItem * item,
    libdnf5::rpm::Nevra nevra,
    libdnf5::rpm::TransactionCallbacks::ScriptType type) {
    output_stream << "  Running " << script_type_to_string(type) << " scriptlet: " << to_full_nevra_string(nevra)
                  << std::endl;
}

void TransactionCallbacksSimple::script_stop(
    [[maybe_unused]] const libdnf5::rpm::TransactionItem * item,
    libdnf5::rpm::Nevra nevra,
    libdnf5::rpm::TransactionCallbacks::ScriptType type,
    [[maybe_unused]] uint64_t return_code) {
    output_stream << "  Stop " << script_type_to_string(type) << " scriptlet: " << to_full_nevra_string(nevra)
                  << std::endl;
}

}  // namespace dnf5
