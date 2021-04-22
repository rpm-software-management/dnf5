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

#include "libdnf/system/state.hpp"

#include "libdnf/utils/fs.hpp"

#include <toml.hpp>
#include <fstream>

namespace libdnf::system {

State::State(
    const std::filesystem::path & installroot,
    const std::filesystem::path & dir_path)
    : path(installroot / dir_path) {
    load();
}


libdnf::transaction::TransactionItemReason State::get_reason(const std::string & na) {
    auto it = reasons.find(na);
    if (it == reasons.end()) {
        return libdnf::transaction::TransactionItemReason::DEPENDENCY;
    }

    return it->second;
}


void State::set_reason(const std::string & na, libdnf::transaction::TransactionItemReason reason) {
    if (reason == libdnf::transaction::TransactionItemReason::DEPENDENCY) {
        reasons.erase(na);
    } else {
        reasons[na] = reason;
    }
}


void State::save() {
    std::filesystem::path path = get_userinstalled_path();
    utils::fs::makedirs_for_file(path);

    std::vector<std::string> userinstalled;

    for (auto & na_reason : reasons) {
        if (na_reason.second == libdnf::transaction::TransactionItemReason::USER) {
            userinstalled.push_back(na_reason.first);
        }
    }
    std::sort(userinstalled.begin(), userinstalled.end());

    std::ofstream toml(path);
    toml << toml::value({{"userinstalled", userinstalled}});
    toml.close();
}


void State::load() {
    std::filesystem::path path = get_userinstalled_path();

    if (!std::filesystem::exists(path)) {
        return;
    }

    // TODO(lukash) throws std::runtime_error with no error description in case opening the file fails
    for (auto & na : toml::find<std::vector<std::string>>(toml::parse(path), "userinstalled")) {
        reasons[na] = libdnf::transaction::TransactionItemReason::USER;
    }
}


std::filesystem::path State::get_userinstalled_path() {
    return path / "userinstalled.toml";
}

}  // namespace libdnf::system
