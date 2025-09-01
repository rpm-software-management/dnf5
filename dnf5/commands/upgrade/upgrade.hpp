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

#ifndef DNF5_COMMANDS_UPGRADE_UPGRADE_HPP
#define DNF5_COMMANDS_UPGRADE_UPGRADE_HPP

#include "../advisory_shared.hpp"

#include <dnf5/context.hpp>
#include <dnf5/shared_options.hpp>
#include <libdnf5/conf/option_bool.hpp>

#include <memory>
#include <vector>

namespace dnf5 {

class UpgradeCommand : public Command {
public:
    explicit UpgradeCommand(Context & context) : Command(context, "upgrade") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

protected:
    libdnf5::OptionBool * minimal{nullptr};
    std::vector<std::string> pkg_specs;

    std::unique_ptr<AllowErasingOption> allow_erasing;

    std::vector<std::string> installed_from_repos;
    std::vector<std::string> from_repos;

    std::unique_ptr<AdvisoryOption> advisory_name;
    std::unique_ptr<SecurityOption> advisory_security;
    std::unique_ptr<BugfixOption> advisory_bugfix;
    std::unique_ptr<EnhancementOption> advisory_enhancement;
    std::unique_ptr<NewpackageOption> advisory_newpackage;
    std::unique_ptr<AdvisorySeverityOption> advisory_severity;
    std::unique_ptr<BzOption> advisory_bz;
    std::unique_ptr<CveOption> advisory_cve;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_UPGRADE_UPGRADE_HPP
