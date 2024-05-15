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

#include "libdnf5/rpm/transaction_callbacks.hpp"

namespace libdnf5::rpm {

const char * TransactionCallbacks::script_type_to_string(ScriptType type) noexcept {
    switch (type) {
        case ScriptType::PRE_INSTALL:
            return "pre-install";
        case ScriptType::POST_INSTALL:
            return "post-install";
        case ScriptType::PRE_UNINSTALL:
            return "pre-uninstall";
        case ScriptType::POST_UNINSTALL:
            return "post-uninstall";
        case ScriptType::PRE_TRANSACTION:
            return "pre-transaction";
        case ScriptType::POST_TRANSACTION:
            return "post-transaction";
        case ScriptType::TRIGGER_PRE_INSTALL:
            return "trigger-pre-install";
        case ScriptType::TRIGGER_INSTALL:
            return "trigger-install";
        case ScriptType::TRIGGER_UNINSTALL:
            return "trigger-uninstall";
        case ScriptType::TRIGGER_POST_UNINSTALL:
            return "trigger-post-uninstall";
        case ScriptType::UNKNOWN:
            return "unknown";
    }
    return "unknown";
}

TransactionCallbacks::TransactionCallbacks() = default;

TransactionCallbacks::~TransactionCallbacks() = default;

void TransactionCallbacks::before_begin(uint64_t) {}

void TransactionCallbacks::after_complete(bool) {}

void TransactionCallbacks::install_progress(const TransactionItem &, uint64_t, uint64_t) {}

void TransactionCallbacks::install_start(const TransactionItem &, uint64_t) {}

void TransactionCallbacks::install_stop(const TransactionItem &, uint64_t, uint64_t) {}

void TransactionCallbacks::transaction_progress(uint64_t, uint64_t) {}

void TransactionCallbacks::transaction_start(uint64_t) {}

void TransactionCallbacks::transaction_stop(uint64_t) {}

void TransactionCallbacks::uninstall_progress(const TransactionItem &, uint64_t, uint64_t) {}

void TransactionCallbacks::uninstall_start(const TransactionItem &, uint64_t) {}

void TransactionCallbacks::uninstall_stop(const TransactionItem &, uint64_t, uint64_t) {}

void TransactionCallbacks::unpack_error(const TransactionItem &) {}

void TransactionCallbacks::cpio_error(const TransactionItem &) {}

void TransactionCallbacks::script_error(const TransactionItem *, Nevra, ScriptType, uint64_t) {}

void TransactionCallbacks::script_start(const TransactionItem *, Nevra, ScriptType) {}

void TransactionCallbacks::script_stop(const TransactionItem *, Nevra, ScriptType, uint64_t) {}

void TransactionCallbacks::elem_progress(const TransactionItem &, uint64_t, uint64_t) {}

void TransactionCallbacks::verify_progress(uint64_t, uint64_t) {}

void TransactionCallbacks::verify_start(uint64_t) {}

void TransactionCallbacks::verify_stop(uint64_t) {}

}  // namespace libdnf5::rpm
