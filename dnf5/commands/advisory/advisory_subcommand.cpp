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

#include <filesystem>
#include <fstream>
#include <set>


namespace dnf5 {


using namespace libdnf::cli;


AdvisorySubCommand::AdvisorySubCommand(
    Command & parent, const std::string & name, const std::string & short_description)
    : Command(parent, name) {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description(short_description);

    all = std::make_unique<AdvisoryAllOption>(*this);
    available = std::make_unique<AdvisoryAvailableOption>(*this);
    installed = std::make_unique<AdvisoryInstalledOption>(*this);
    updates = std::make_unique<AdvisoryUpdatesOption>(*this);
    advisory_specs = std::make_unique<AdvisorySpecArguments>(*this);
    contains_pkgs = std::make_unique<AdvisoryContainsPkgsOption>(*this);

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

void AdvisorySubCommand::run() {
    auto & ctx = get_context();

    ctx.load_repos(true, libdnf::repo::Repo::LoadFlags::UPDATEINFO);

    libdnf::rpm::PackageQuery package_query(ctx.base);
    auto package_specs_strs = contains_pkgs->get_value();
    // Filter packages by name patterns if given
    if (package_specs_strs.size() > 0) {
        package_query.filter_name(package_specs_strs, libdnf::sack::QueryCmp::IGLOB);
    }

    auto advisories = libdnf::advisory::AdvisoryQuery(ctx.base);
    auto advisory_specs_strs = advisory_specs->get_value();
    // Filter advisories by patterns if given
    if (advisory_specs_strs.size() > 0) {
        advisories.filter_name(advisory_specs_strs, libdnf::sack::QueryCmp::IGLOB);
    }

    process_and_print_queries(ctx, advisories, package_query);
}

}  // namespace dnf5
