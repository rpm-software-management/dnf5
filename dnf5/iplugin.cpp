// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "dnf5/iplugin.hpp"

namespace dnf5 {

IPlugin::IPlugin(Context & context) : context(&context) {}

IPlugin::~IPlugin() = default;

void IPlugin::init() {}

Context & IPlugin::get_context() const noexcept {
    return *context;
}

}  // namespace dnf5
