// Copyright Contributors to the DNF5 project.
// Copyright (C) 2024 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "libdnf5/common/exception.hpp"

#include <libdnf5/transaction/offline.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/fs/file.hpp>
#include <toml.hpp>


const int STATE_VERSION = 1;
const std::string STATE_HEADER{"offline-transaction-state"};


struct OfflineTransactionStateTomlData {
    int state_version = STATE_VERSION;
    std::string status = libdnf5::offline::STATUS_DOWNLOAD_INCOMPLETE;
    std::string cachedir;
    std::string target_releasever;
    std::string system_releasever;
    std::string verb;
    std::string cmd_line;
    bool poweroff_after = false;
    std::string module_platform_id;
};

TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(
    OfflineTransactionStateTomlData,
    state_version,
    status,
    cachedir,
    target_releasever,
    system_releasever,
    verb,
    cmd_line,
    poweroff_after,
    module_platform_id)

namespace libdnf5::offline {

class OfflineTransactionStateData::Impl {
public:
    friend OfflineTransactionStateData;
    OfflineTransactionStateTomlData data;
};


OfflineTransactionStateData::~OfflineTransactionStateData() = default;

OfflineTransactionStateData::OfflineTransactionStateData() : p_impl(std::make_unique<Impl>()) {};
OfflineTransactionStateData::OfflineTransactionStateData(const OfflineTransactionStateData & src)
    : p_impl(new Impl(*src.p_impl)) {}
OfflineTransactionStateData::OfflineTransactionStateData(OfflineTransactionStateData && src) noexcept = default;

OfflineTransactionStateData & OfflineTransactionStateData::operator=(const OfflineTransactionStateData & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }
    return *this;
}

OfflineTransactionStateData & OfflineTransactionStateData::operator=(OfflineTransactionStateData && src) noexcept =
    default;

void OfflineTransactionStateData::set_state_version(int state_version) {
    p_impl->data.state_version = state_version;
}
int OfflineTransactionStateData::get_state_version() const {
    return p_impl->data.state_version;
}

void OfflineTransactionStateData::set_status(const std::string & status) {
    p_impl->data.status = status;
}
const std::string & OfflineTransactionStateData::get_status() const {
    return p_impl->data.status;
}

void OfflineTransactionStateData::set_cachedir(const std::string & cachedir) {
    p_impl->data.cachedir = cachedir;
}
const std::string & OfflineTransactionStateData::get_cachedir() const {
    return p_impl->data.cachedir;
}

void OfflineTransactionStateData::set_target_releasever(const std::string & target_releasever) {
    p_impl->data.target_releasever = target_releasever;
}
const std::string & OfflineTransactionStateData::get_target_releasever() const {
    return p_impl->data.target_releasever;
}

void OfflineTransactionStateData::set_system_releasever(const std::string & system_releasever) {
    p_impl->data.system_releasever = system_releasever;
}
const std::string & OfflineTransactionStateData::get_system_releasever() const {
    return p_impl->data.system_releasever;
}

void OfflineTransactionStateData::set_verb(const std::string & verb) {
    p_impl->data.verb = verb;
}
const std::string & OfflineTransactionStateData::get_verb() const {
    return p_impl->data.verb;
}

void OfflineTransactionStateData::set_cmd_line(const std::string & cmd_line) {
    p_impl->data.cmd_line = cmd_line;
}
const std::string & OfflineTransactionStateData::get_cmd_line() const {
    return p_impl->data.cmd_line;
}

void OfflineTransactionStateData::set_poweroff_after(bool poweroff_after) {
    p_impl->data.poweroff_after = poweroff_after;
}
bool OfflineTransactionStateData::get_poweroff_after() const {
    return p_impl->data.poweroff_after;
}

void OfflineTransactionStateData::set_module_platform_id(const std::string & module_platform_id) {
    p_impl->data.module_platform_id = module_platform_id;
}
const std::string & OfflineTransactionStateData::get_module_platform_id() const {
    return p_impl->data.module_platform_id;
}


class OfflineTransactionState::Impl {
    friend OfflineTransactionState;
    std::exception_ptr read_exception;
    std::filesystem::path path;
    OfflineTransactionStateData data;
};

OfflineTransactionState::~OfflineTransactionState() = default;

OfflineTransactionState::OfflineTransactionState(std::filesystem::path path) : p_impl(std::make_unique<Impl>()) {
    p_impl->path = std::move(path);
    read();
}
OfflineTransactionState::OfflineTransactionState(const OfflineTransactionState & src) : p_impl(new Impl(*src.p_impl)) {}
OfflineTransactionState::OfflineTransactionState(OfflineTransactionState && src) noexcept = default;

OfflineTransactionState & OfflineTransactionState::operator=(const OfflineTransactionState & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }
    return *this;
}
OfflineTransactionState & OfflineTransactionState::operator=(OfflineTransactionState && src) noexcept = default;

OfflineTransactionStateData & OfflineTransactionState::get_data() {
    return p_impl->data;
}

const std::exception_ptr & OfflineTransactionState::get_read_exception() const {
    return p_impl->read_exception;
}

std::filesystem::path OfflineTransactionState::get_path() const {
    return p_impl->path;
}


void OfflineTransactionState::read() {
    try {
        const std::ifstream file{p_impl->path};
        if (!file.good()) {
            throw libdnf5::FileSystemError(errno, p_impl->path, M_("error reading offline state file"));
        }
        const auto & value = toml::parse(p_impl->path);
        p_impl->data.p_impl->data = toml::find<OfflineTransactionStateTomlData>(value, STATE_HEADER);
        if (p_impl->data.get_state_version() != STATE_VERSION) {
            throw libdnf5::RuntimeError(M_("incompatible version of state data"));
        }
    } catch (const std::exception & ex) {
        p_impl->read_exception = std::current_exception();
        p_impl->data = OfflineTransactionStateData{};
    }
}

void OfflineTransactionState::write() {
    auto file = libdnf5::utils::fs::File(p_impl->path, "w");
#ifdef TOML11_COMPAT
    file.write(toml::format(toml::value{{STATE_HEADER, p_impl->data.p_impl->data}}));
#else
    file.write(toml::format(toml::value{toml::table{{STATE_HEADER, p_impl->data.p_impl->data}}}));
#endif
    file.close();
}

}  // namespace libdnf5::offline
