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


#ifndef LIBDNF5_CLI_OUTPUT_ADAPTERS_COMPS_TMPL_HPP
#define LIBDNF5_CLI_OUTPUT_ADAPTERS_COMPS_TMPL_HPP

#include "../interfaces/comps.hpp"

namespace libdnf5::cli::output {

template <class T>
class GroupPackageAdapter : public IGroupPackage {
public:
    GroupPackageAdapter(const T & obj) : obj{obj} {}

    GroupPackageAdapter(T && obj) : obj{std::move(obj)} {}

    std::string get_name() const override { return obj.get_name(); }

    libdnf5::comps::PackageType get_type() const override { return obj.get_type(); }

private:
    T obj;
};


template <class T>
class GroupAdapter : public IGroup {
public:
    GroupAdapter(const T & grp) : grp{grp} {}

    GroupAdapter(T && grp) : grp{std::move(grp)} {}

    std::string get_groupid() const override { return grp.get_groupid(); }

    std::string get_name() const override { return grp.get_name(); }

    std::vector<std::unique_ptr<IGroupPackage>> get_packages() override {
        std::vector<std::unique_ptr<IGroupPackage>> ret;
        auto packages = grp.get_packages();
        ret.reserve(packages.size());
        for (auto & package : packages) {
            ret.emplace_back(new GroupPackageAdapter(package));
        }
        return ret;
    }

    std::string get_description() const override { return grp.get_description(); }

    std::string get_order() const override { return grp.get_order(); }

    int get_order_int() const override { return grp.get_order_int(); }

    std::string get_langonly() const override { return grp.get_langonly(); }

    bool get_uservisible() const override { return grp.get_uservisible(); }

    std::set<std::string> get_repos() const override { return grp.get_repos(); }

    bool get_installed() const override { return grp.get_installed(); }

    libdnf5::transaction::TransactionItemReason get_reason() const override { return grp.get_reason(); }

private:
    T grp;
};


template <class T>
class EnvironmentAdapter : public IEnvironment {
public:
    EnvironmentAdapter(const T & env) : env{env} {}

    EnvironmentAdapter(T && env) : env{std::move(env)} {}

    std::string get_environmentid() const override { return env.get_environmentid(); }

    std::string get_name() const override { return env.get_name(); }

    std::string get_description() const override { return env.get_description(); }

    std::string get_order() const override { return env.get_order(); }

    int get_order_int() const override { return env.get_order_int(); }

    std::vector<std::string> get_groups() override {
        if constexpr (requires { env.get_groups(); }) {
            return env.get_groups();
        } else {
            return {};
        }
    }

    std::vector<std::string> get_optional_groups() override {
        if constexpr (requires { env.get_groups(); }) {
            return env.get_optional_groups();
        } else {
            return {};
        }
    }

    std::set<std::string> get_repos() const override { return env.get_repos(); }

    bool get_installed() const override { return env.get_installed(); }

private:
    T env;
};

template <class T>
bool comps_display_order_cmp(T & a, T & b) {
    return a.get_order_int() < b.get_order_int();
}

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_ADAPTERS_COMPS_TMPL_HPP
