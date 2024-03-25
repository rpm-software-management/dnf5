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
#include <dnf5/offline.hpp>

namespace dnf5 {

class SystemUpgradeCommand : public Command {
public:
    explicit SystemUpgradeCommand(Context & context) : Command(context, "system-upgrade") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void register_subcommands() override;
    void pre_configure() override;
};

class SystemUpgradeDownloadCommand : public Command {
public:
    explicit SystemUpgradeDownloadCommand(Context & context) : Command{context, "download"} {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    libdnf5::OptionBool * no_downgrade{nullptr};
    std::filesystem::path datadir{dnf5::offline::DEFAULT_DATADIR};
    std::string target_releasever;
    std::string system_releasever;
};

class OfflineDistroSyncCommand : public Command {
public:
    explicit OfflineDistroSyncCommand(Context & context) : Command(context, "offline-distrosync") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void register_subcommands() override;
    void pre_configure() override;
};

class OfflineUpgradeCommand : public Command {
public:
    explicit OfflineUpgradeCommand(Context & context) : Command(context, "offline-upgrade") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void register_subcommands() override;
    void pre_configure() override;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_SYSTEM_UPGRADE_HPP
