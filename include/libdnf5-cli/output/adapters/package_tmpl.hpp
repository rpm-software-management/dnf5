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


#ifndef LIBDNF5_CLI_OUTPUT_ADAPTERS_PACKAGE_TMPL_HPP
#define LIBDNF5_CLI_OUTPUT_ADAPTERS_PACKAGE_TMPL_HPP

#include "../interfaces/package.hpp"

namespace libdnf5::cli::output {

template <class T>
class PackageAdapter : public IPackage {
public:
    PackageAdapter(const T & pkg) : pkg{pkg} {}

    PackageAdapter(T && pkg) : pkg{std::move(pkg)} {}

    std::string get_name() const override { return pkg.get_name(); }

    std::string get_epoch() const override { return pkg.get_epoch(); }

    std::string get_version() const override { return pkg.get_version(); }

    std::string get_release() const override { return pkg.get_release(); }

    std::string get_arch() const override { return pkg.get_arch(); }

    std::string get_evr() const override { return pkg.get_evr(); }

    std::string get_nevra() const override { return pkg.get_nevra(); }

    std::string get_full_nevra() const override { return pkg.get_full_nevra(); }

    std::string get_na() const override { return pkg.get_na(); }

    unsigned long long get_download_size() const override { return pkg.get_download_size(); }

    unsigned long long get_install_size() const override { return pkg.get_install_size(); }

    std::string get_license() const override { return pkg.get_license(); }

    std::string get_sourcerpm() const override { return pkg.get_sourcerpm(); }

    std::string get_vendor() const override { return pkg.get_vendor(); }

    std::string get_url() const override { return pkg.get_url(); }

    std::string get_summary() const override { return pkg.get_summary(); }

    std::string get_description() const override { return pkg.get_description(); }

    std::vector<std::string> get_files() const override {
        if constexpr (requires { pkg.get_files(); }) {
            return pkg.get_files();
        } else {
            return {};
        }
    }

    bool is_installed() const override { return pkg.is_installed(); }

    std::string get_from_repo_id() const override { return pkg.get_from_repo_id(); }

    std::string get_repo_id() const override { return pkg.get_repo_id(); }

    libdnf5::transaction::TransactionItemReason get_reason() const override { return pkg.get_reason(); }

private:
    T pkg;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_ADAPTERS_PACKAGE_TMPL_HPP
