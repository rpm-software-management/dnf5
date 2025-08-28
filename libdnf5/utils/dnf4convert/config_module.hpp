// Copyright Contributors to the DNF5 project.
// Copyright (C) 2022 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later

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
