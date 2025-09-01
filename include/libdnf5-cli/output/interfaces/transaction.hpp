// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#ifndef LIBDNF5_CLI_OUTPUT_INTERFACES_TRANSACTION_HPP
#define LIBDNF5_CLI_OUTPUT_INTERFACES_TRANSACTION_HPP

#include <libdnf5/transaction/transaction_item_action.hpp>
#include <libdnf5/transaction/transaction_item_reason.hpp>

#include <memory>
#include <string>
#include <vector>

namespace libdnf5::cli::output {

class IEnvironment;
class IGroup;
class IPackage;

class ITransactionPackage {
public:
    virtual ~ITransactionPackage() = default;

    virtual std::unique_ptr<IPackage> get_package() const = 0;
    virtual libdnf5::transaction::TransactionItemAction get_action() const = 0;
    virtual libdnf5::transaction::TransactionItemReason get_reason() const = 0;
    virtual std::vector<std::unique_ptr<IPackage>> get_replaces() const = 0;
};


class ITransactionGroup {
public:
    virtual ~ITransactionGroup() = default;

    virtual libdnf5::transaction::TransactionItemAction get_action() const = 0;
    virtual libdnf5::transaction::TransactionItemReason get_reason() const = 0;
    virtual std::unique_ptr<IGroup> get_group() const = 0;
};


class ITransactionEnvironment {
public:
    virtual ~ITransactionEnvironment() = default;

    virtual libdnf5::transaction::TransactionItemAction get_action() const = 0;
    virtual libdnf5::transaction::TransactionItemReason get_reason() const = 0;
    virtual std::unique_ptr<IEnvironment> get_environment() const = 0;
};


class ITransactionModule {
public:
    virtual ~ITransactionModule() = default;

    virtual libdnf5::transaction::TransactionItemAction get_action() const = 0;
    virtual libdnf5::transaction::TransactionItemReason get_reason() const = 0;
    virtual std::string get_module_name() const = 0;
    virtual std::string get_module_stream() const = 0;
    virtual std::vector<std::pair<std::string, std::string>> get_replaces() const = 0;
};


class ITransaction {
public:
    virtual ~ITransaction() = default;

    virtual std::vector<std::string> get_resolve_logs_as_strings() const = 0;
    virtual std::vector<std::unique_ptr<ITransactionPackage>> get_transaction_packages() const = 0;
    virtual std::vector<std::unique_ptr<IPackage>> get_conflicting_packages() const = 0;
    virtual std::vector<std::unique_ptr<IPackage>> get_broken_dependency_packages() const = 0;
    virtual std::vector<std::unique_ptr<ITransactionGroup>> get_transaction_groups() const = 0;
    virtual std::vector<std::unique_ptr<ITransactionModule>> get_transaction_modules() const = 0;
    virtual std::vector<std::unique_ptr<ITransactionEnvironment>> get_transaction_environments() const = 0;
    virtual bool empty() const = 0;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_INTERFACES_TRANSACTION_HPP
