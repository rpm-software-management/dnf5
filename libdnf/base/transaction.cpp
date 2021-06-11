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


#include "libdnf/base/transaction.hpp"

#include <solv/transaction.h>

namespace libdnf::base {


class Transaction::Impl {
public:
    Impl(const BaseWeakPtr & base) : base(base) {}
    Impl(const Impl & src)
        : base(src.base),
          libsolv_transaction(src.libsolv_transaction ? transaction_create_clone(src.libsolv_transaction) : nullptr),
          packages(src.packages) {}
    ~Impl();

    Impl & operator=(const Impl & other);

private:
    friend Transaction;

    BaseWeakPtr base;
    ::Transaction * libsolv_transaction{nullptr};

    std::vector<TransactionPackageItem> packages;
};

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

}  // namespace libdnf::base
