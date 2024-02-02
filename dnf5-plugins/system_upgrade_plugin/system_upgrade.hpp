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
#include <libdnf5/conf/const.hpp>
#include <libdnf5/conf/option_bool.hpp>
#include <libdnf5/conf/option_number.hpp>
#include <sdbus-c++/sdbus-c++.h>
#include <toml.hpp>

const std::filesystem::path PATH_TO_PLYMOUTH{"/usr/bin/plymouth"};
const std::filesystem::path PATH_TO_JOURNALCTL{"/usr/bin/journalctl"};
const std::string OFFLINE_STARTED_ID{"3e0a5636d16b4ca4bbe5321d06c6aa62"};
const std::string OFFLINE_FINISHED_ID{"8cec00a1566f4d3594f116450395f06c"};

const std::string STATUS_DOWNLOAD_INCOMPLETE{"download-incomplete"};
const std::string STATUS_DOWNLOAD_COMPLETE{"download-complete"};
const std::string STATUS_READY{"ready"};
const std::string STATUS_UPGRADE_INCOMPLETE{"upgrade-incomplete"};

const int STATE_VERSION = 0;
const std::string STATE_HEADER{"offline-transaction-state"};

struct OfflineTransactionStateData {
    int state_version = STATE_VERSION;
    std::string status = STATUS_DOWNLOAD_INCOMPLETE;
    std::string cachedir;
    std::string target_releasever;
    std::string system_releasever;
    bool poweroff_after = false;
    std::vector<std::string> enabled_repos;
    std::vector<std::string> disabled_repos;
};

TOML11_DEFINE_CONVERSION_NON_INTRUSIVE(
    OfflineTransactionStateData,
    state_version,
    status,
    cachedir,
    target_releasever,
    system_releasever,
    poweroff_after,
    enabled_repos,
    disabled_repos)

class OfflineTransactionState {
public:
    void write();
    OfflineTransactionState(std::filesystem::path path);
    OfflineTransactionStateData & get_data() { return data; };
    const std::exception_ptr & get_read_exception() const { return read_exception; };

private:
    void read();
    std::exception_ptr read_exception;
    std::filesystem::path path;
    OfflineTransactionStateData data;
};

const std::filesystem::path DEFAULT_DATADIR{std::filesystem::path(libdnf5::SYSTEM_STATE_DIR) / "system-upgrade"};

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
    OfflineTransactionState get_state() const { return state; };
    std::string get_system_releasever() const { return system_releasever; };
    std::string get_target_releasever() const { return target_releasever; };
    void log_status(const std::string & message, const std::string & message_id) const;

private:
    libdnf5::OptionPath * cachedir{nullptr};
    std::filesystem::path datadir{DEFAULT_DATADIR};
    std::filesystem::path magic_symlink;
    OfflineTransactionState state;
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
    libdnf5::OptionBool * distro_sync{nullptr};
    int state_version;
};

class SystemUpgradeRebootCommand : public SystemUpgradeSubcommand {
public:
    explicit SystemUpgradeRebootCommand(Context & context) : SystemUpgradeSubcommand(context, "reboot") {}
    void set_argument_parser() override;
    void run() override;

private:
    libdnf5::OptionBool * poweroff_after{nullptr};
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

private:
    libdnf5::OptionString * number{nullptr};
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_CHANGELOG_HPP
