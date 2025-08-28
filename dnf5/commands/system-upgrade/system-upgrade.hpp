// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_SYSTEM_UPGRADE_HPP
#define DNF5_COMMANDS_SYSTEM_UPGRADE_HPP

#include <dnf5/context.hpp>
#include <dnf5/shared_options.hpp>
#include <libdnf5/transaction/offline.hpp>

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
    std::unique_ptr<AllowErasingOption> allow_erasing;
    std::filesystem::path datadir{libdnf5::offline::DEFAULT_DATADIR};
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
