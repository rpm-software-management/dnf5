// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_INSTALL_INSTALL_HPP
#define DNF5_COMMANDS_INSTALL_INSTALL_HPP

#include "../advisory_shared.hpp"

#include <dnf5/context.hpp>
#include <dnf5/shared_options.hpp>
#include <libdnf5/rpm/package.hpp>

#include <memory>
#include <vector>

namespace dnf5 {

class InstallCommand : public Command {
public:
    explicit InstallCommand(Context & context) : Command(context, "install") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    std::vector<std::string> pkg_specs;

    std::unique_ptr<AllowErasingOption> allow_erasing;

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


#endif  // DNF5_COMMANDS_INSTALL_INSTALL_HPP
