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


#include "advisory_info.hpp"

#include "../../context.hpp"

#include "libdnf-cli/utils/tty.hpp"

#include <libdnf-cli/output/advisoryinfo.hpp>
#include <libdnf/rpm/package_query.hpp>

#include <filesystem>
#include <fstream>
#include <set>


namespace microdnf {


using namespace libdnf::cli;


AdvisoryInfoCommand::AdvisoryInfoCommand(Command & parent) : AdvisoryInfoCommand(parent, "info") {}


AdvisoryInfoCommand::AdvisoryInfoCommand(Command & parent, const std::string & name) : Command(parent, name) {
    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("Info advisories");

    available = std::make_unique<AdvisoryAvailableOption>(*this);
    installed = std::make_unique<AdvisoryInstalledOption>(*this);
    all = std::make_unique<AdvisoryAllOption>(*this);
    updates = std::make_unique<AdvisoryUpdatesOption>(*this);
    // TODO(amatej): set_conflicting_args({available, installed, all, updates});

    package_specs = std::make_unique<AdvisorySpecArguments>(*this);
}


void AdvisoryInfoCommand::run() {
    auto & ctx = static_cast<Context &>(get_session());

    ctx.base.get_repo_sack()->get_system_repo()->load();

    libdnf::repo::RepoQuery enabled_repos(ctx.base);
    enabled_repos.filter_enabled(true);

    using QueryCmp = libdnf::sack::QueryCmp;

    ctx.load_rpm_repos(enabled_repos, libdnf::repo::Repo::LoadFlags::UPDATEINFO);

    libdnf::rpm::PackageQuery package_query(ctx.base);

    auto package_specs_str = package_specs->get_value();

    // Filter by patterns if given
    if (package_specs_str.size() > 0) {
        package_query.filter_name(package_specs_str, QueryCmp::IGLOB);
    }

    auto adv_q_installed = libdnf::advisory::AdvisoryQuery(ctx.base);
    auto adv_q_available = libdnf::advisory::AdvisoryQuery(ctx.base);
    auto adv_q_updates = libdnf::advisory::AdvisoryQuery(ctx.base);

    if (installed->get_value() || all->get_value()) {
        auto installed_package_query = package_query.filter_installed();
        adv_q_installed.filter_packages(installed_package_query, QueryCmp::LTE);
    }

    // Default if nothing specified
    if (available->get_value() || all->get_value() ||
        (!installed->get_value() && !available->get_value() && !updates->get_value())) {
        auto installed_package_query = package_query.filter_installed().filter_latest_evr();
        //TODO(amatej): https://github.com/rpm-software-management/dnf/pull/1485, show for currently running
        //kernel and if it is not the latests one show also for the latest kernel
        //auto kernel_id = ctx.base.get_rpm_package_sack()->p_impl->get_running_kernel();
        //Use rpm::PackageId Goal::get_running_kernel_internal()?

        adv_q_updates.filter_packages(installed_package_query, QueryCmp::GT);
    }

    if (updates->get_value()) {
        auto upgradable_package_query = package_query.filter_upgradable();
        adv_q_updates.filter_packages(upgradable_package_query, QueryCmp::GT);
    }

    adv_q_installed |= adv_q_available;
    adv_q_installed |= adv_q_updates;

    for (auto advisory : adv_q_installed) {
        libdnf::cli::output::AdvisoryInfo advisory_info;
        advisory_info.add_advisory(advisory);
        advisory_info.print();
        std::cout << std::endl;
    }
}


}  // namespace microdnf
