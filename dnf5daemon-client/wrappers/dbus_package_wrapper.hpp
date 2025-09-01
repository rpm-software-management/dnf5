// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef DNF5DAEMON_CLIENT_WRAPPER_DBUS_PACKAGE_WRAPPER_HPP
#define DNF5DAEMON_CLIENT_WRAPPER_DBUS_PACKAGE_WRAPPER_HPP

#include <dnf5daemon-server/dbus.hpp>
#include <libdnf5/transaction/transaction_item_reason.hpp>

#include <vector>


namespace dnfdaemon::client {

class DbusPackageWrapper {
public:
    explicit DbusPackageWrapper(const dnfdaemon::KeyValueMap & rawdata) : rawdata(rawdata) {};

    int get_id() { return int{rawdata.at("id")}; }
    std::string get_name() const { return std::string{rawdata.at("name")}; }
    std::string get_na() const { return get_name() + "." + get_arch(); }
    std::string get_epoch() const { return std::string{rawdata.at("epoch")}; }
    std::string get_version() const { return std::string{rawdata.at("version")}; }
    std::string get_release() const { return std::string{rawdata.at("release")}; }
    std::string get_arch() const { return std::string{rawdata.at("arch")}; }
    std::string get_repo_id() const { return std::string{rawdata.at("repo_id")}; }
    std::string get_from_repo_id() const { return std::string{rawdata.at("from_repo_id")}; }
    std::string get_nevra() const { return std::string{rawdata.at("nevra")}; }
    std::string get_full_nevra() const { return std::string{rawdata.at("full_nevra")}; }
    std::string get_evr() const { return std::string{rawdata.at("evr")}; }
    bool is_installed() const { return bool{rawdata.at("is_installed")}; }
    uint64_t get_install_size() const { return uint64_t{rawdata.at("install_size")}; }
    uint64_t get_download_size() const { return uint64_t{rawdata.at("download_size")}; }
    std::string get_sourcerpm() const { return std::string{rawdata.at("sourcerpm")}; }
    std::string get_summary() const { return std::string{rawdata.at("summary")}; }
    std::string get_url() const { return std::string{rawdata.at("url")}; }
    std::string get_license() const { return std::string{rawdata.at("license")}; }
    std::string get_description() const { return std::string{rawdata.at("description")}; }
    libdnf5::transaction::TransactionItemReason get_reason() const {
        return libdnf5::transaction::transaction_item_reason_from_string(std::string{rawdata.at("reason")});
    }
    std::string get_vendor() const { return std::string{rawdata.at("vendor")}; }

private:
    dnfdaemon::KeyValueMap rawdata;
};

}  // namespace dnfdaemon::client

#endif  // DNF5DAEMON_CLIENT_WRAPPER_DBUS_PACKAGE_WRAPPER_HPP
