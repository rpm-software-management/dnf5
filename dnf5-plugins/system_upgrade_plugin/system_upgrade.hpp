/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DNF5_COMMANDS_SYSTEM_UPGRADE_HPP
#define DNF5_COMMANDS_SYSTEM_UPGRADE_HPP

#include <dnf5/context.hpp>
#include <libdnf5/conf/option_bool.hpp>
#include <libdnf5/conf/option_number.hpp>

/* #include <map> */
/* #include <any> */

/* class State { */
/* public: */
/*     State(std::filesystem::path path); */
/* private: */
/*     void read(); */
/*     void write(); */
/*     std::map<std::string, std::any> data; */
/*     std::filesystem::path path; */
/* }; */

namespace dnf5 {

class SystemUpgradeCommand : public Command {
public:
    explicit SystemUpgradeCommand(Context & context) : Command(context, "system-upgrade") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void register_subcommands() override;
    void pre_configure() override;
};

class SystemUpgradeSubcommand : public Command {
public:
    explicit SystemUpgradeSubcommand(Context & context, const std::string & name);
    void set_argument_parser() override;
    void configure() override;

protected:
    libdnf5::OptionPath * get_download_dir() { return download_dir; };
    std::filesystem::path get_data_dir() { return data_dir; };
    std::filesystem::path get_magic_symlink() { return magic_symlink; };

private:
    libdnf5::OptionPath * download_dir{nullptr};
    std::filesystem::path data_dir;
    std::filesystem::path magic_symlink;
};

class SystemUpgradeDownloadCommand : public SystemUpgradeSubcommand {
public:
    explicit SystemUpgradeDownloadCommand(Context & context) : SystemUpgradeSubcommand(context, "download") {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    libdnf5::OptionBool * no_downgrade{nullptr};
    libdnf5::OptionBool * distro_sync{nullptr};
};

class SystemUpgradeRebootCommand : public SystemUpgradeSubcommand {
public:
    explicit SystemUpgradeRebootCommand(Context & context) : SystemUpgradeSubcommand(context, "reboot") {}
    void set_argument_parser() override;
    void run() override;
};

class SystemUpgradeUpgradeCommand : public SystemUpgradeSubcommand {
public:
    explicit SystemUpgradeUpgradeCommand(Context & context) : SystemUpgradeSubcommand(context, "upgrade") {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;
};

class SystemUpgradeCleanCommand : public SystemUpgradeSubcommand {
public:
    explicit SystemUpgradeCleanCommand(Context & context) : SystemUpgradeSubcommand(context, "clean") {}
    void set_argument_parser() override;
    void run() override;
};

class SystemUpgradeLogCommand : public SystemUpgradeSubcommand {
public:
    explicit SystemUpgradeLogCommand(Context & context) : SystemUpgradeSubcommand(context, "log") {}
    void set_argument_parser() override;
    void run() override;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_CHANGELOG_HPP
