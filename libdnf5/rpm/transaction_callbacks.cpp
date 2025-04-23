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
            return "%pre";
        case ScriptType::POST_INSTALL:
            return "%post";
        case ScriptType::PRE_UNINSTALL:
            return "%preun";
        case ScriptType::POST_UNINSTALL:
            return "%postun";
        case ScriptType::PRE_TRANSACTION:
            return "%pretrans";
        case ScriptType::POST_TRANSACTION:
            return "%posttrans";
        case ScriptType::TRIGGER_PRE_INSTALL:
            return "%triggerprein";
        case ScriptType::TRIGGER_INSTALL:
            return "%triggerin";
        case ScriptType::TRIGGER_UNINSTALL:
            return "%triggerun";
        case ScriptType::TRIGGER_POST_UNINSTALL:
            return "%triggerpostun";
        case ScriptType::SYSUSERS:
            return "sysusers";
        case ScriptType::PREUN_TRANSACTION:
            return "%preuntrans";
        case ScriptType::POSTUN_TRANSACTION:
            return "%postuntrans";
        case ScriptType::UNKNOWN:
            return "unknown";
    }
    return "unknown";
}

TransactionCallbacks::TransactionCallbacks() = default;

TransactionCallbacks::~TransactionCallbacks() = default;

void TransactionCallbacks::before_begin(uint64_t) {}

void TransactionCallbacks::after_complete(bool) {}

void TransactionCallbacks::install_progress(const libdnf5::base::TransactionPackage &, uint64_t, uint64_t) {}

void TransactionCallbacks::install_start(const libdnf5::base::TransactionPackage &, uint64_t) {}

void TransactionCallbacks::install_stop(const libdnf5::base::TransactionPackage &, uint64_t, uint64_t) {}

void TransactionCallbacks::transaction_progress(uint64_t, uint64_t) {}

void TransactionCallbacks::transaction_start(uint64_t) {}

void TransactionCallbacks::transaction_stop(uint64_t) {}

void TransactionCallbacks::uninstall_progress(const libdnf5::base::TransactionPackage &, uint64_t, uint64_t) {}

void TransactionCallbacks::uninstall_start(const libdnf5::base::TransactionPackage &, uint64_t) {}

void TransactionCallbacks::uninstall_stop(const libdnf5::base::TransactionPackage &, uint64_t, uint64_t) {}

void TransactionCallbacks::unpack_error(const libdnf5::base::TransactionPackage &) {}

void TransactionCallbacks::cpio_error(const libdnf5::base::TransactionPackage &) {}

void TransactionCallbacks::script_error(const libdnf5::base::TransactionPackage *, Nevra, ScriptType, uint64_t) {}

void TransactionCallbacks::script_start(const libdnf5::base::TransactionPackage *, Nevra, ScriptType) {}

void TransactionCallbacks::script_stop(const libdnf5::base::TransactionPackage *, Nevra, ScriptType, uint64_t) {}

void TransactionCallbacks::elem_progress(const libdnf5::base::TransactionPackage &, uint64_t, uint64_t) {}

void TransactionCallbacks::verify_progress(uint64_t, uint64_t) {}

void TransactionCallbacks::verify_start(uint64_t) {}

void TransactionCallbacks::verify_stop(uint64_t) {}

}  // namespace libdnf5::rpm
