// Copyright (C) 2022 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/dnf5/
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

#ifndef LIBDNF5_UTILS_DNF4CONVERT_CONFIG_MODULE_HPP
#define LIBDNF5_UTILS_DNF4CONVERT_CONFIG_MODULE_HPP

#include "libdnf5/conf/config.hpp"
#include "libdnf5/conf/option_enum.hpp"
#include "libdnf5/conf/option_string.hpp"
#include "libdnf5/conf/option_string_list.hpp"

#include <map>
#include <string>
#include <vector>

namespace libdnf5::dnf4convert {

class ConfigModule : public Config {
public:
    explicit ConfigModule(const std::string & module_name);
    ~ConfigModule() = default;

    OptionString name{""};
    OptionString stream{""};
    OptionStringList profiles{std::vector<std::string>{}};
    OptionEnum state{"", {"enabled", "disabled", ""}};

    void load_from_parser(
        const libdnf5::ConfigParser & parser,
        const std::string & section,
        const libdnf5::Vars & vars,
        libdnf5::Logger & logger,
        Option::Priority priority = Option::Priority::DEFAULT) override {
        Config::load_from_parser(parser, section, vars, logger, priority);
    }

private:
    std::string module_name;
};

}  // namespace libdnf5::dnf4convert

#endif
