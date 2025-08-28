// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_ADVISORY_ADVISORY_SUBCOMMAND_HPP
#define DNF5_COMMANDS_ADVISORY_ADVISORY_SUBCOMMAND_HPP

#include "../advisory_shared.hpp"
#include "arguments.hpp"

#include <dnf5/context.hpp>
#include <libdnf5/advisory/advisory_query.hpp>

#include <memory>

namespace dnf5 {

class AdvisorySubCommand : public Command {
public:
    void set_argument_parser() override;
    void configure() override;
    void run() override;

protected:
    virtual void process_and_print_queries(
        Context & ctx,
        libdnf5::advisory::AdvisoryQuery & advisories,
        const std::vector<std::string> & package_specs) = 0;

    void add_running_kernel_packages(libdnf5::Base & base, libdnf5::rpm::PackageQuery & package_query);

    std::unique_ptr<AdvisoryAvailableOption> available{nullptr};
    std::unique_ptr<AdvisoryInstalledOption> installed{nullptr};
    std::unique_ptr<AdvisoryAllOption> all{nullptr};
    std::unique_ptr<AdvisoryUpdatesOption> updates{nullptr};
    std::unique_ptr<AdvisoryContainsPkgsOption> contains_pkgs{nullptr};
    std::unique_ptr<AdvisorySpecArguments> advisory_specs{nullptr};
    std::unique_ptr<AdvisoryWithBzOption> with_bz{nullptr};
    std::unique_ptr<AdvisoryWithCveOption> with_cve{nullptr};

    AdvisorySubCommand(Context & context, const std::string & name) : Command(context, name) {}

protected:
    std::unique_ptr<SecurityOption> advisory_security{nullptr};
    std::unique_ptr<BugfixOption> advisory_bugfix{nullptr};
    std::unique_ptr<EnhancementOption> advisory_enhancement{nullptr};
    std::unique_ptr<NewpackageOption> advisory_newpackage{nullptr};
    std::unique_ptr<AdvisorySeverityOption> advisory_severity{nullptr};
    std::unique_ptr<BzOption> advisory_bz{nullptr};
    std::unique_ptr<CveOption> advisory_cve{nullptr};
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_ADVISORY_ADVISORY_SUBCOMMAND_HPP
