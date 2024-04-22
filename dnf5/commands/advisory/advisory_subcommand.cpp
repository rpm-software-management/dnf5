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

#include "advisory_subcommand.hpp"

#include "dnf5/context.hpp"

#include <libdnf5-cli/output/advisorysummary.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/rpm/package_query.hpp>

namespace dnf5 {

using namespace libdnf5::cli;

void AdvisorySubCommand::set_argument_parser() {
    all = std::make_unique<AdvisoryAllOption>(*this);
    available = std::make_unique<AdvisoryAvailableOption>(*this);
    installed = std::make_unique<AdvisoryInstalledOption>(*this);
    updates = std::make_unique<AdvisoryUpdatesOption>(*this);
    advisory_specs = std::make_unique<AdvisorySpecArguments>(*this);
    contains_pkgs = std::make_unique<AdvisoryContainsPkgsOption>(*this);

    advisory_security = std::make_unique<SecurityOption>(*this);
    advisory_bugfix = std::make_unique<BugfixOption>(*this);
    advisory_enhancement = std::make_unique<EnhancementOption>(*this);
    advisory_newpackage = std::make_unique<NewpackageOption>(*this);
    advisory_severity = std::make_unique<AdvisorySeverityOption>(*this);
    advisory_bz = std::make_unique<BzOption>(*this);
    advisory_cve = std::make_unique<CveOption>(*this);

    with_bz = std::make_unique<AdvisoryWithBzOption>(*this);
    with_cve = std::make_unique<AdvisoryWithCveOption>(*this);

    all->get_arg()->add_conflict_argument(*available->get_arg());
    all->get_arg()->add_conflict_argument(*installed->get_arg());
    all->get_arg()->add_conflict_argument(*updates->get_arg());
    available->get_arg()->add_conflict_argument(*installed->get_arg());
    available->get_arg()->add_conflict_argument(*updates->get_arg());
    installed->get_arg()->add_conflict_argument(*updates->get_arg());
}

// There can be multiple versions of kernel installed at the same time.
// When the running kernel has available advisories show them because the system
// is vulnerable (even if the newer fixed version of kernel is already installed).
// DNF4 bug with behavior description: https://bugzilla.redhat.com/show_bug.cgi?id=1728004
void AdvisorySubCommand::add_running_kernel_packages(libdnf5::Base & base, libdnf5::rpm::PackageQuery & package_query) {
    auto kernel = base.get_rpm_package_sack()->get_running_kernel();
    if (kernel.get_id().id > 0) {
        libdnf5::rpm::PackageQuery kernel_query(base);
        kernel_query.filter_sourcerpm({kernel.get_sourcerpm()});
        kernel_query.filter_installed();
        package_query |= kernel_query;
    }
}

void AdvisorySubCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
    context.base.get_config().get_optional_metadata_types_option().add_item(
        libdnf5::Option::Priority::RUNTIME, libdnf5::METADATA_TYPE_UPDATEINFO);
}

void AdvisorySubCommand::run() {
    auto & ctx = get_context();

    auto advisories_opt = advisory_query_from_cli_input(
        ctx.base,
        advisory_specs->get_value(),
        advisory_security->get_value(),
        advisory_bugfix->get_value(),
        advisory_enhancement->get_value(),
        advisory_newpackage->get_value(),
        advisory_severity->get_value(),
        advisory_bz->get_value(),
        advisory_cve->get_value());

    auto advisories = advisories_opt.value_or(libdnf5::advisory::AdvisoryQuery(ctx.base));

    if (with_bz->get_value()) {
        advisories.filter_reference("*", {"bugzilla"}, libdnf5::sack::QueryCmp::IGLOB);
    }
    if (with_cve->get_value()) {
        advisories.filter_reference("*", {"cve"}, libdnf5::sack::QueryCmp::IGLOB);
    }

    process_and_print_queries(ctx, advisories, contains_pkgs->get_value());
}

}  // namespace dnf5
