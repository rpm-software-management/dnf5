// Copyright Contributors to the DNF5 project.
// Copyright (C) 2022 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "config_module.hpp"

namespace libdnf5::dnf4convert {

ConfigModule::ConfigModule(const std::string & module_name) : module_name(module_name) {
    opt_binds().add("name", name);
    opt_binds().add("stream", stream);
    opt_binds().add("profiles", profiles);
    opt_binds().add("state", state);
}


}  // namespace libdnf5::dnf4convert
