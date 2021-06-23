/*
Copyright (C) 2021 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef LIBDNF_BASE_TRANSACTION_HPP
#define LIBDNF_BASE_TRANSACTION_HPP

#include "libdnf/base/goal_elements.hpp"
#include "libdnf/base/transaction_package.hpp"

#include <optional>


namespace libdnf {

class Base;
using BaseWeakPtr = WeakPtr<Base, false>;

}  // namespace libdnf


namespace libdnf::base {

class Transaction {
public:
    Transaction(const Transaction & transaction);
    ~Transaction();

    libdnf::GoalProblem get_problems();

    /// @return the transaction packages.
    std::vector<libdnf::base::TransactionPackage> get_packages();

private:
    friend class libdnf::Goal;

    Transaction(const BaseWeakPtr & base);

    class Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf::base

#endif  // LIBDNF_BASE_TRANSACTION_HPP
