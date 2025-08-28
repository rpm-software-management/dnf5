// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "iplugin_private.hpp"

namespace libdnf5::plugin {


StopRequest::StopRequest() noexcept = default;
StopRequest::StopRequest(const StopRequest & other) noexcept = default;
StopRequest::StopRequest(StopRequest && other) noexcept = default;
StopRequest::~StopRequest() = default;
StopRequest & StopRequest::operator=(const StopRequest & other) noexcept = default;
StopRequest & StopRequest::operator=(StopRequest && other) noexcept = default;


class IPlugin::Impl {
public:
    explicit Impl(IPluginData & data) : data(&data) {}

    Base & get_base() const noexcept { return plugin::get_base(*data); }

private:
    IPluginData * data;
};


IPlugin::IPlugin(IPluginData & data) : p_impl(new Impl(data)) {}

IPlugin::~IPlugin() = default;

void IPlugin::load_plugins() {}

void IPlugin::init() {}

void IPlugin::pre_base_setup() {}

void IPlugin::post_base_setup() {}

void IPlugin::repos_configured() {}

void IPlugin::repos_loaded() {}

void IPlugin::pre_add_cmdline_packages([[maybe_unused]] const std::vector<std::string> & paths) {}

void IPlugin::post_add_cmdline_packages() {}

void IPlugin::pre_transaction([[maybe_unused]] const libdnf5::base::Transaction & transaction) {}

void IPlugin::post_transaction([[maybe_unused]] const libdnf5::base::Transaction & transaction) {}

void IPlugin::finish() noexcept {}

Base & IPlugin::get_base() const noexcept {
    return p_impl->get_base();
}

IPlugin2_1::IPlugin2_1(IPluginData & data) : IPlugin(data) {}

IPlugin2_1::~IPlugin2_1() = default;

void IPlugin2_1::goal_resolved([[maybe_unused]] const libdnf5::base::Transaction & transaction) {}

}  // namespace libdnf5::plugin
