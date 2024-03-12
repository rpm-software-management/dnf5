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

#ifndef DNF5_COMMANDS_OFFLINE_HPP
#define DNF5_COMMANDS_OFFLINE_HPP

#include <dnf5/context.hpp>
#include <dnf5/offline.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/conf/option_bool.hpp>
#include <libdnf5/conf/option_number.hpp>
#include <sdbus-c++/sdbus-c++.h>
#include <toml.hpp>

const std::filesystem::path PATH_TO_PLYMOUTH{"/usr/bin/plymouth"};
const std::filesystem::path PATH_TO_JOURNALCTL{"/usr/bin/journalctl"};

namespace dnf5 {

class OfflineCommand : public Command {
public:
    explicit OfflineCommand(Context & context) : Command(context, "offline") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void register_subcommands() override;
    void pre_configure() override;
};

class OfflineSubcommand : public Command {
public:
    explicit OfflineSubcommand(Context & context, const std::string & name);
    void configure() override;

protected:
    std::filesystem::path get_magic_symlink() const { return magic_symlink; };
    std::filesystem::path get_datadir() const { return datadir; };
    std::optional<dnf5::offline::OfflineTransactionState> state;
    std::string get_system_releasever() const { return system_releasever; };
    std::string get_target_releasever() const { return target_releasever; };

private:
    std::filesystem::path magic_symlink;
    std::filesystem::path datadir;
    std::string target_releasever;
    std::string system_releasever;
};

class OfflineRebootCommand : public OfflineSubcommand {
public:
    explicit OfflineRebootCommand(Context & context) : OfflineSubcommand(context, "reboot") {}
    void set_argument_parser() override;
    void run() override;

private:
    libdnf5::OptionBool * poweroff_after{nullptr};
};

class OfflineExecuteCommand : public OfflineSubcommand {
public:
    explicit OfflineExecuteCommand(Context & context) : OfflineSubcommand(context, "_execute") {}
    void set_argument_parser() override;
    void pre_configure() override;
    void configure() override;
    void run() override;
};

class OfflineCleanCommand : public OfflineSubcommand {
public:
    explicit OfflineCleanCommand(Context & context) : OfflineSubcommand(context, "clean") {}
    void set_argument_parser() override;
    void run() override;
};

class OfflineLogCommand : public OfflineSubcommand {
public:
    explicit OfflineLogCommand(Context & context) : OfflineSubcommand(context, "log") {}
    void set_argument_parser() override;
    void run() override;

private:
    libdnf5::OptionString * number{nullptr};
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_OFFLINE_HPP
