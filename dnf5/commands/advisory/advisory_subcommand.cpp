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

#include <libdnf-cli/output/advisorysummary.hpp>
#include <libdnf/rpm/package_query.hpp>

namespace dnf5 {

using namespace libdnf::cli;

void AdvisorySubCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

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

    auto conflict_args = parser.add_conflict_args_group(std::unique_ptr<std::vector<ArgumentParser::Argument *>>(
        new std::vector<ArgumentParser::Argument *>{all->arg, available->arg, installed->arg, updates->arg}));

    all->arg->set_conflict_arguments(conflict_args);
    available->arg->set_conflict_arguments(conflict_args);
    installed->arg->set_conflict_arguments(conflict_args);
    updates->arg->set_conflict_arguments(conflict_args);
}

// There can be multiple versions of kernel installed at the same time.
// When the running kernel has available advisories show them because the system
// is vulnerable (even if the newer fixed version of kernel is already installed).
// DNF4 bug with behavior description: https://bugzilla.redhat.com/show_bug.cgi?id=1728004
void AdvisorySubCommand::add_running_kernel_packages(libdnf::Base & base, libdnf::rpm::PackageQuery & package_query) {
    auto kernel = base.get_rpm_package_sack()->get_running_kernel();
    if (kernel.get_id().id > 0) {
        libdnf::rpm::PackageQuery kernel_query(base);
        kernel_query.filter_sourcerpm({kernel.get_sourcerpm()});
        kernel_query.filter_installed();
        package_query |= kernel_query;
    }
}

void AdvisorySubCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
    context.set_available_repos_load_flags(libdnf::repo::LoadFlags::PRIMARY | libdnf::repo::LoadFlags::UPDATEINFO);
}

void AdvisorySubCommand::run() {
    auto & ctx = get_context();

    libdnf::rpm::PackageQuery package_query(ctx.base);
    auto package_specs_strs = contains_pkgs->get_value();
    // Filter packages by name patterns if given
    if (package_specs_strs.size() > 0) {
        package_query.filter_name(package_specs_strs, libdnf::sack::QueryCmp::IGLOB);
    }

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

    auto advisories = advisories_opt.value_or(libdnf::advisory::AdvisoryQuery(ctx.base));

    if (with_bz->get_value()) {
        advisories.filter_reference("*", libdnf::sack::QueryCmp::IGLOB, {"bugzilla"});
    }
    if (with_cve->get_value()) {
        advisories.filter_reference("*", libdnf::sack::QueryCmp::IGLOB, {"cve"});
    }

    process_and_print_queries(ctx, advisories, package_query);
}

}  // namespace dnf5
