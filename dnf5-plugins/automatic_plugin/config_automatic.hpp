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

#ifndef DNF5_PLUGINS_AUTOMATIC_PLUGIN_CONFIG_AUTOMATIC_HPP
#define DNF5_PLUGINS_AUTOMATIC_PLUGIN_CONFIG_AUTOMATIC_HPP

#include "libdnf5/conf/config.hpp"
#include "libdnf5/conf/option_bool.hpp"
#include "libdnf5/conf/option_enum.hpp"
#include "libdnf5/conf/option_number.hpp"
#include "libdnf5/conf/option_string.hpp"
#include "libdnf5/conf/option_string_list.hpp"

#include <map>
#include <string>
#include <vector>

namespace dnf5 {

// options in [commands] section
class ConfigAutomaticCommands : public libdnf5::Config {
public:
    ConfigAutomaticCommands();
    ~ConfigAutomaticCommands() = default;

    libdnf5::OptionEnum<std::string> upgrade_type{"default", {"default", "security"}};
    libdnf5::OptionNumber<std::int32_t> random_sleep{0};
    libdnf5::OptionNumber<std::int32_t> network_online_timeout{60};
    libdnf5::OptionBool download_updates{true};
    libdnf5::OptionBool apply_updates{false};
    libdnf5::OptionEnum<std::string> reboot{"never", {"never", "when-changed", "when-needed"}};
    libdnf5::OptionString reboot_command{"shutdown -r +5 'Rebooting after applying package updates'"};
};


// options in [emitters] section
class ConfigAutomaticEmitters : public libdnf5::Config {
public:
    ConfigAutomaticEmitters();
    ~ConfigAutomaticEmitters() = default;

    libdnf5::OptionStringList emit_via{std::vector<std::string>{"stdio"}};
    libdnf5::OptionString system_name{gethostname()};

private:
    static std::string gethostname();
};


// options in [email] section
class ConfigAutomaticEmail : public libdnf5::Config {
public:
    ConfigAutomaticEmail();
    ~ConfigAutomaticEmail() = default;

    libdnf5::OptionStringList email_to{std::vector<std::string>{"root"}};
    libdnf5::OptionString email_from{"root"};
    libdnf5::OptionString email_host{"localhost"};
    libdnf5::OptionNumber<std::int32_t> email_port{25};
    libdnf5::OptionEnum<std::string> email_tls{"no", {"no", "yes", "starttls"}};
};


// options in [command] section
class ConfigAutomaticCommand : public libdnf5::Config {
public:
    ConfigAutomaticCommand();
    ~ConfigAutomaticCommand() = default;

    libdnf5::OptionString command_format{"cat"};
    libdnf5::OptionString stdin_format{"{body}"};
};


// options in [command_email] section
class ConfigAutomaticCommandEmail : public libdnf5::Config {
public:
    ConfigAutomaticCommandEmail();
    ~ConfigAutomaticCommandEmail() = default;

    libdnf5::OptionString command_format{"mail -Ssendwait -s {subject} -r {email_from} {email_to}"};
    libdnf5::OptionString stdin_format{"{body}"};
    libdnf5::OptionStringList email_to{std::vector<std::string>{"root"}};
    libdnf5::OptionString email_from{"root"};
};


class ConfigAutomatic {
public:
    ConfigAutomatic(){};
    ~ConfigAutomatic() = default;

    void load_from_parser(
        const libdnf5::ConfigParser & parser,
        const libdnf5::Vars & vars,
        libdnf5::Logger & logger,
        libdnf5::Option::Priority priority = libdnf5::Option::Priority::AUTOMATICCONFIG);

    libdnf5::OptionString automatic_config_file_path{"/etc/dnf/automatic.conf"};

    ConfigAutomaticCommands config_commands;
    ConfigAutomaticEmitters config_emitters;
    ConfigAutomaticEmail config_email;
    ConfigAutomaticCommand config_command;
    ConfigAutomaticCommandEmail config_command_email;
};

}  // namespace dnf5

#endif
