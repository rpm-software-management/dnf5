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

#include "plugin_info_impl.hpp"

namespace libdnf5::plugin {

PluginInfo::PluginInfo(Impl & p_impl) : p_impl{&p_impl} {}

PluginInfo::~PluginInfo() = default;

PluginInfo::PluginInfo(const PluginInfo & src) = default;

PluginInfo::PluginInfo(PluginInfo && src) noexcept = default;

PluginInfo & PluginInfo::operator=(const PluginInfo & src) = default;

PluginInfo & PluginInfo::operator=(PluginInfo && src) noexcept = default;

const std::string & PluginInfo::get_name() const noexcept {
    return p_impl->get_name();
}

bool PluginInfo::is_loaded() const noexcept {
    return p_impl->is_loaded();
}

PluginAPIVersion PluginInfo::get_api_version() const noexcept {
    return p_impl->get_api_version();
}

const char * PluginInfo::get_real_name() const noexcept {
    return p_impl->get_real_name();
}

Version PluginInfo::get_version() const noexcept {
    return p_impl->get_version();
}

const char * const * PluginInfo::get_attributes() const noexcept {
    return p_impl->get_attributes();
}

const char * PluginInfo::get_attribute(const char * name) const noexcept {
    return p_impl->get_attribute(name);
}

}  // namespace libdnf5::plugin
