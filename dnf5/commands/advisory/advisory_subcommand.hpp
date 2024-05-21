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
    std::unique_ptr<AdvisoryListAsXmlOption> as_xml{nullptr};

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
