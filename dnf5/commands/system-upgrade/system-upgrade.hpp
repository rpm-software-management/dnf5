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
#include <libdnf5/offline/offline.hpp>

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
    libdnf5::OptionPath * get_cachedir() const { return cachedir; };
    std::filesystem::path get_datadir() const { return datadir; };
    std::filesystem::path get_magic_symlink() const { return magic_symlink; };
    libdnf5::offline::OfflineTransactionState get_state() const { return state; };
    std::string get_system_releasever() const { return system_releasever; };
    std::string get_target_releasever() const { return target_releasever; };
    void log_status(const std::string & message, const std::string & message_id) const;

private:
    libdnf5::OptionPath * cachedir{nullptr};
    std::filesystem::path datadir{libdnf5::offline::DEFAULT_DATADIR};
    std::filesystem::path magic_symlink;
    libdnf5::offline::OfflineTransactionState state;
    std::string target_releasever;
    std::string system_releasever;
};

class SystemUpgradeDownloadCommand : public SystemUpgradeSubcommand {
public:
    explicit SystemUpgradeDownloadCommand(Context & context) : SystemUpgradeSubcommand{context, "download"} {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    libdnf5::OptionBool * no_downgrade{nullptr};
    int state_version;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_SYSTEM_UPGRADE_HPP
