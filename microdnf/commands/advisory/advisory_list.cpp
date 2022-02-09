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


#include "advisory_list.hpp"

#include "context.hpp"

#include "libdnf-cli/output/advisorylist.hpp"

#include <libdnf/rpm/package_query.hpp>

#include <filesystem>
#include <fstream>


namespace microdnf {


using namespace libdnf::cli;


AdvisoryListCommand::AdvisoryListCommand(Command & parent) : AdvisoryListCommand(parent, "list") {}


AdvisoryListCommand::AdvisoryListCommand(Command & parent, const std::string & name) : Command(parent, name) {
    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("List advisories");

    available = std::make_unique<AdvisoryAvailableOption>(*this);
    installed = std::make_unique<AdvisoryInstalledOption>(*this);
    all = std::make_unique<AdvisoryAllOption>(*this);
    updates = std::make_unique<AdvisoryUpdatesOption>(*this);
    // TODO(amatej): set_conflicting_args({available, installed, all, updates});

    package_specs = std::make_unique<AdvisorySpecArguments>(*this);
}


//TODO(amatej): the run is duplicated over all advisory commands?
void AdvisoryListCommand::run() {
    auto & ctx = static_cast<Context &>(get_session());

    ctx.load_repos(true, libdnf::repo::Repo::LoadFlags::UPDATEINFO);

    using QueryCmp = libdnf::sack::QueryCmp;

    libdnf::rpm::PackageQuery package_query(ctx.base);

    auto package_specs_str = package_specs->get_value();

    // Filter by patterns if given
    if (package_specs_str.size() > 0) {
        package_query.filter_name(package_specs_str, QueryCmp::IGLOB);
    }

    auto advisory_query = libdnf::advisory::AdvisoryQuery(ctx.base);

    std::vector<libdnf::advisory::AdvisoryPackage> pkgs;
    std::vector<libdnf::advisory::AdvisoryPackage> installed_pkgs;

    if (installed->get_value() || all->get_value()) {
        auto installed_package_query = package_query;
        installed_package_query.filter_installed();
        installed_pkgs = advisory_query.get_advisory_packages(installed_package_query, QueryCmp::LTE);
    }

    // Default if nothing specified
    if (available->get_value() || all->get_value() ||
        (!installed->get_value() && !available->get_value() && !updates->get_value())) {
        auto installed_package_query = package_query;
        installed_package_query.filter_installed();
        installed_package_query.filter_latest_evr();
        //TODO(amatej): https://github.com/rpm-software-management/dnf/pull/1485, show for currently running
        //kernel and if it is not the latests one show also for the latest kernel
        //auto kernel_id = ctx.base.get_rpm_package_sack()->p_impl->get_running_kernel();
        //Use rpm::PackageId Goal::get_running_kernel_internal()?

        pkgs = advisory_query.get_advisory_packages(installed_package_query, QueryCmp::GT);
    }

    if (updates->get_value()) {
        auto upgradable_package_query = package_query;
        upgradable_package_query.filter_upgradable();
        pkgs = advisory_query.get_advisory_packages(upgradable_package_query, QueryCmp::GT);
    }

    libdnf::cli::output::print_advisorylist_table(pkgs, installed_pkgs);
}


}  // namespace microdnf
