// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef DNF5_COMMANDS_ADVISORY_ADVISORY_SUBCOMMAND_HPP
#define DNF5_COMMANDS_ADVISORY_ADVISORY_SUBCOMMAND_HPP

#include "../../wrappers/dbus_advisory_wrapper.hpp"
#include "arguments.hpp"
#include "commands/command.hpp"
#include "context.hpp"

#include <memory>

namespace dnfdaemon::client {

class AdvisorySubCommand : public DaemonCommand {
public:
    void set_argument_parser() override;
    dnfdaemon::KeyValueMap session_config() override;
    void run() override;

protected:
    AdvisorySubCommand(Context & context, const std::string & name) : DaemonCommand(context, name) {}
    virtual void process_and_print_queries(const std::vector<DbusAdvisoryWrapper> & advisories) = 0;

    std::unique_ptr<AdvisoryAvailableOption> available{nullptr};
    std::unique_ptr<AdvisoryInstalledOption> installed{nullptr};
    std::unique_ptr<AdvisoryAllOption> all{nullptr};
    std::unique_ptr<AdvisoryUpdatesOption> updates{nullptr};
    std::unique_ptr<AdvisoryContainsPkgsOption> contains_pkgs{nullptr};
    std::unique_ptr<AdvisoryNameArguments> advisory_names{nullptr};
    std::unique_ptr<AdvisoryWithBzOption> with_bz{nullptr};
    std::unique_ptr<AdvisoryWithCveOption> with_cve{nullptr};

    std::unique_ptr<SecurityOption> advisory_security{nullptr};
    std::unique_ptr<BugfixOption> advisory_bugfix{nullptr};
    std::unique_ptr<EnhancementOption> advisory_enhancement{nullptr};
    std::unique_ptr<NewpackageOption> advisory_newpackage{nullptr};
    std::unique_ptr<AdvisorySeverityOption> advisory_severities{nullptr};
    std::unique_ptr<BzOption> advisory_bzs{nullptr};
    std::unique_ptr<CveOption> advisory_cves{nullptr};

    std::vector<std::string> advisory_attrs{};
};

}  // namespace dnfdaemon::client

#endif  // DNF5_COMMANDS_ADVISORY_ADVISORY_SUBCOMMAND_HPP
