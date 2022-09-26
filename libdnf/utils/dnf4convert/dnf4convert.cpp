/*
Copyright (C) 2022 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/dnf5/

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

#include "dnf4convert.hpp"

#include "config_module.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/conf/config_parser.hpp"

#include <filesystem>

namespace libdnf::dnf4convert {


std::map<std::string, libdnf::system::ModuleState> Dnf4Convert::read_module_states() {
    std::map<std::string, libdnf::system::ModuleState> module_states;

    auto & config = base->get_config();
    std::filesystem::path path = config.installroot().get_value() / std::filesystem::path{"etc/dnf/modules.d"};
    auto & logger = *base->get_logger();
    for (const auto & dir_entry : std::filesystem::directory_iterator(path)) {
        auto & module_file = dir_entry.path();
        if ((dir_entry.is_regular_file() || dir_entry.is_symlink()) && (module_file.extension() == ".module")) {
            logger.debug("Loading module state from file \"{}\"", module_file.string());
            libdnf::ConfigParser parser;
            parser.read(module_file);
            auto d = parser.get_data();
            // each module file contains only one section named by the module name
            auto name = module_file.stem();
            ConfigModule module_config{name};
            module_config.load_from_parser(parser, name, *base->get_vars(), logger);
            libdnf::system::ModuleState state;
            state.enabled_stream = module_config.stream.get_value();
            state.installed_profiles = module_config.profiles.get_value();
            state.state = libdnf::module::ModuleState::AVAILABLE;
            auto status_value = module_config.state.get_value();
            if (status_value == "enabled") {
                state.state = libdnf::module::ModuleState::ENABLED;
            } else if (status_value == "disabled") {
                state.state = libdnf::module::ModuleState::DISABLED;
            }
            module_states.emplace(name, state);
        }
    }
    return module_states;
}


}  // namespace libdnf::dnf4convert
