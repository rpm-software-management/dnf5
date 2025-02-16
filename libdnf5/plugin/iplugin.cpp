/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

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
