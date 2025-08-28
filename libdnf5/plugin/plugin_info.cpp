// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
