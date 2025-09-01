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


#ifndef LIBDNF5_CLI_OUTPUT_ADAPTERS_TRANSACTION_TMPL_HPP
#define LIBDNF5_CLI_OUTPUT_ADAPTERS_TRANSACTION_TMPL_HPP

#include "../interfaces/transaction.hpp"
#include "comps_tmpl.hpp"
#include "package_tmpl.hpp"

namespace libdnf5::cli::output {

template <class T>
class TransactionPackageAdapter : public ITransactionPackage {
public:
    TransactionPackageAdapter(const T & trans_package) : trans_package{trans_package} {}

    TransactionPackageAdapter(T && trans_package) : trans_package{std::move(trans_package)} {}

    std::unique_ptr<IPackage> get_package() const override {
        return std::unique_ptr<IPackage>(new PackageAdapter(trans_package.get_package()));
    }

    libdnf5::transaction::TransactionItemAction get_action() const override { return trans_package.get_action(); }

    libdnf5::transaction::TransactionItemReason get_reason() const override { return trans_package.get_reason(); }

    std::vector<std::unique_ptr<IPackage>> get_replaces() const override {
        std::vector<std::unique_ptr<IPackage>> ret;
        auto replaces = trans_package.get_replaces();
        ret.reserve(replaces.size());
        for (auto & replace : replaces) {
            ret.emplace_back(new PackageAdapter(replace));
        }
        return ret;
    }

private:
    T trans_package;
};


template <class T>
class TransactionGroupAdapter : public ITransactionGroup {
public:
    TransactionGroupAdapter(const T & trans_grp) : trans_grp{trans_grp} {}

    TransactionGroupAdapter(T && trans_grp) : trans_grp{std::move(trans_grp)} {}

    libdnf5::transaction::TransactionItemAction get_action() const override { return trans_grp.get_action(); }

    libdnf5::transaction::TransactionItemReason get_reason() const override { return trans_grp.get_reason(); }

    std::unique_ptr<IGroup> get_group() const override {
        return std::unique_ptr<IGroup>(new GroupAdapter(trans_grp.get_group()));
    }

private:
    T trans_grp;
};


template <class T>
class TransactionEnvironmentAdapter : public ITransactionEnvironment {
public:
    TransactionEnvironmentAdapter(const T & trans_env) : trans_env{trans_env} {}

    TransactionEnvironmentAdapter(T && trans_env) : trans_env{std::move(trans_env)} {}

    libdnf5::transaction::TransactionItemAction get_action() const override { return trans_env.get_action(); }

    libdnf5::transaction::TransactionItemReason get_reason() const override { return trans_env.get_reason(); }

    std::unique_ptr<IEnvironment> get_environment() const override {
        return std::unique_ptr<IEnvironment>(new EnvironmentAdapter(trans_env.get_environment()));
    }

private:
    T trans_env;
};


template <class T>
class TransactionModuleAdapter : public ITransactionModule {
public:
    TransactionModuleAdapter(const T & trans_module) : trans_module{trans_module} {}

    TransactionModuleAdapter(T && trans_module) : trans_module{std::move(trans_module)} {}

    libdnf5::transaction::TransactionItemAction get_action() const override { return trans_module.get_action(); }

    libdnf5::transaction::TransactionItemReason get_reason() const override { return trans_module.get_reason(); }

    std::string get_module_name() const override { return trans_module.get_module_name(); }

    std::string get_module_stream() const override { return trans_module.get_module_stream(); }

    std::vector<std::pair<std::string, std::string>> get_replaces() const override {
        return trans_module.get_replaces();
    }

private:
    T trans_module;
};


template <class T>
class TransactionAdapter : public ITransaction {
public:
    TransactionAdapter(const T & transaction) : transaction{&transaction} {}

    std::vector<std::string> get_resolve_logs_as_strings() const override {
        return transaction->get_resolve_logs_as_strings();
    }

    std::vector<std::unique_ptr<ITransactionPackage>> get_transaction_packages() const override {
        std::vector<std::unique_ptr<ITransactionPackage>> ret;
        const auto & trans_packages = transaction->get_transaction_packages();
        ret.reserve(trans_packages.size());
        for (auto & trans_pkg : trans_packages) {
            //ret.push_back(std::make_unique<TransactionPackageAdapter<decltype(trans_pkg)>>(trans_pkg));
            ret.emplace_back(new TransactionPackageAdapter(trans_pkg));
        }
        return ret;
    }

    std::vector<std::unique_ptr<IPackage>> get_conflicting_packages() const override {
        std::vector<std::unique_ptr<IPackage>> ret;
        const auto & conflicting_packages = transaction->get_conflicting_packages();
        ret.reserve(conflicting_packages.size());
        for (auto & pkg : conflicting_packages) {
            ret.emplace_back(new PackageAdapter(pkg));
        }
        return ret;
    }

    std::vector<std::unique_ptr<IPackage>> get_broken_dependency_packages() const override {
        std::vector<std::unique_ptr<IPackage>> ret;
        const auto & broken_packages = transaction->get_broken_dependency_packages();
        ret.reserve(broken_packages.size());
        for (auto & pkg : broken_packages) {
            ret.emplace_back(new PackageAdapter(pkg));
        }
        return ret;
    }

    std::vector<std::unique_ptr<ITransactionGroup>> get_transaction_groups() const override {
        std::vector<std::unique_ptr<ITransactionGroup>> ret;
        const auto & trans_groups = transaction->get_transaction_groups();
        ret.reserve(trans_groups.size());
        for (auto & trans_grp : trans_groups) {
            ret.emplace_back(new TransactionGroupAdapter(trans_grp));
        }
        return ret;
    }

    std::vector<std::unique_ptr<ITransactionModule>> get_transaction_modules() const override {
        std::vector<std::unique_ptr<ITransactionModule>> ret;
        const auto & trans_modules = transaction->get_transaction_modules();
        ret.reserve(trans_modules.size());
        for (auto & trans_module : trans_modules) {
            ret.emplace_back(new TransactionModuleAdapter(trans_module));
        }
        return ret;
    }

    std::vector<std::unique_ptr<ITransactionEnvironment>> get_transaction_environments() const override {
        std::vector<std::unique_ptr<ITransactionEnvironment>> ret;
        const auto & trans_envs = transaction->get_transaction_environments();
        ret.reserve(trans_envs.size());
        for (auto & trans_env : trans_envs) {
            ret.emplace_back(new TransactionEnvironmentAdapter(trans_env));
        }
        return ret;
    }

    bool empty() const override { return transaction->empty(); }

private:
    const T * transaction;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_ADAPTERS_TRANSACTION_TMPL_HPP
