/*
Copyright (C) 2021 Red Hat, Inc.

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


#include "transaction_impl.hpp"

#include "libdnf/base/base.hpp"


namespace libdnf::base {

Transaction::Transaction(const BaseWeakPtr & base) : p_impl(new Impl(base)) {}
Transaction::Transaction(const Transaction & transaction) : p_impl(new Impl(*transaction.p_impl)) {}
Transaction::~Transaction() = default;

Transaction::Impl::~Impl() {
    if (libsolv_transaction) {
        transaction_free(libsolv_transaction);
    }
}

Transaction::Impl & Transaction::Impl::operator=(const Impl & other) {
    base = other.base;
    libsolv_transaction = other.libsolv_transaction ? transaction_create_clone(other.libsolv_transaction) : nullptr;
    packages = other.packages;
    return *this;
}

const std::vector<TransactionPackageItem> & Transaction::get_packages() const noexcept {
    return p_impl->packages;
}

GoalProblem Transaction::get_problems() {
    return p_impl->problems;
}

std::vector<rpm::Package> Transaction::Impl::list_results(Id type_filter1, Id type_filter2) {
    /* no transaction */
    if (!libsolv_transaction) {
        throw std::runtime_error("no solution possible");
    }

    auto sack = base->get_rpm_package_sack();

    std::vector<rpm::Package> result;
    const int common_mode = SOLVER_TRANSACTION_SHOW_OBSOLETES | SOLVER_TRANSACTION_CHANGE_IS_REINSTALL;

    for (int i = 0; i < libsolv_transaction->steps.count; ++i) {
        Id p = libsolv_transaction->steps.elements[i];
        Id type;

        switch (type_filter1) {
            case SOLVER_TRANSACTION_OBSOLETED:
                type = transaction_type(libsolv_transaction, p, common_mode);
                break;
            default:
                type = transaction_type(
                    libsolv_transaction, p, common_mode | SOLVER_TRANSACTION_SHOW_ACTIVE | SOLVER_TRANSACTION_SHOW_ALL);
                break;
        }

        if (type == type_filter1 || (type_filter2 && type == type_filter2)) {
            result.emplace_back(rpm::Package(sack, rpm::PackageId(p)));
        }
    }
    return result;
}

std::vector<rpm::Package> Transaction::list_rpm_installs() {
    return p_impl->list_results(SOLVER_TRANSACTION_INSTALL, SOLVER_TRANSACTION_OBSOLETES);
}

std::vector<rpm::Package> Transaction::list_rpm_reinstalls() {
    return p_impl->list_results(SOLVER_TRANSACTION_REINSTALL, 0);
}

std::vector<rpm::Package> Transaction::list_rpm_upgrades() {
    return p_impl->list_results(SOLVER_TRANSACTION_UPGRADE, 0);
}

std::vector<rpm::Package> Transaction::list_rpm_downgrades() {
    return p_impl->list_results(SOLVER_TRANSACTION_DOWNGRADE, 0);
}

std::vector<rpm::Package> Transaction::list_rpm_removes() {
    return p_impl->list_results(SOLVER_TRANSACTION_ERASE, 0);
}

std::vector<rpm::Package> Transaction::list_rpm_obsoleted() {
    return p_impl->list_results(SOLVER_TRANSACTION_OBSOLETED, 0);
}


}  // namespace libdnf::base
