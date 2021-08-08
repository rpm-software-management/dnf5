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


#include "advisory.hpp"

#include "../../context.hpp"

#include <libdnf/conf/option_string.hpp>
#include <libdnf/rpm/package_query.hpp>

//TODO(amatej): just for test output -> remove
#include <iostream>


namespace microdnf {


using namespace libdnf::cli;


AdvisoryCommand::AdvisoryCommand(Command & parent) : Command(parent, "advisory") {
    auto & ctx = static_cast<Context &>(get_session());
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    availability_option = dynamic_cast<libdnf::OptionEnum<std::string> *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionEnum<std::string>>(
            new libdnf::OptionEnum<std::string>("available", {"all", "available", "installed", "updates"}))));

    auto all = parser.add_new_named_arg("all");
    all->set_long_name("all");
    all->set_short_description("show advisories about any version of installed packages");
    all->set_const_value("all");
    all->link_value(availability_option);

    auto available = parser.add_new_named_arg("available");
    available->set_long_name("available");
    available->set_short_description("show advisories about newer versions of installed packages (default)");
    available->set_const_value("available");
    available->link_value(availability_option);

    auto installed = parser.add_new_named_arg("installed");
    installed->set_long_name("installed");
    installed->set_short_description("show advisories about equal and older versions of installed packages");
    installed->set_const_value("installed");
    installed->link_value(availability_option);

    auto updates = parser.add_new_named_arg("updates");
    updates->set_long_name("updates");
    updates->set_short_description(
        "show advisories about newer versions of installed packages for which a newer version is available");
    updates->set_const_value("updates");
    updates->link_value(availability_option);

    auto conflict_args =
        parser.add_conflict_args_group(std::unique_ptr<std::vector<ArgumentParser::Argument *>>(
            new std::vector<ArgumentParser::Argument *>{all, available, installed, updates}));

    all->set_conflict_arguments(conflict_args);
    available->set_conflict_arguments(conflict_args);
    installed->set_conflict_arguments(conflict_args);
    updates->set_conflict_arguments(conflict_args);


    patterns_to_show_options = parser.add_new_values();
    auto specs = parser.add_new_positional_arg(
        "specs_to_show",
        ArgumentParser::PositionalArg::UNLIMITED,
        parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_to_show_options);
    specs->set_short_description("List of specs to use when searching for advisory");


    output_type_option = dynamic_cast<libdnf::OptionEnum<std::string> *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionEnum<std::string>>(
            new libdnf::OptionEnum<std::string>("summary", {"summary", "list", "info"}))));

    auto summary = parser.add_new_named_arg("summary");
    summary->set_long_name("summary");
    summary->set_short_description("show just counts of advisory types (default)");
    summary->set_const_value("summary");
    summary->link_value(output_type_option);

    auto list = parser.add_new_named_arg("list");
    list->set_long_name("list");
    list->set_short_description("show list of advisories");
    list->set_const_value("list");
    list->link_value(output_type_option);

    auto info = parser.add_new_named_arg("info");
    info->set_long_name("info");
    info->set_short_description("show detailed information about advisories");
    info->set_const_value("info");
    info->link_value(output_type_option);

    conflict_args = parser.add_conflict_args_group(std::unique_ptr<std::vector<ArgumentParser::Argument *>>(
        new std::vector<ArgumentParser::Argument *>{summary, list, info}));

    summary->set_conflict_arguments(conflict_args);
    list->set_conflict_arguments(conflict_args);
    info->set_conflict_arguments(conflict_args);


    with_cve_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));
    auto with_cve = parser.add_new_named_arg("with_cve");
    with_cve->set_long_name("with_cve");
    with_cve->set_short_description("show only advisories with CVE reference");
    with_cve->set_const_value("false");
    with_cve->link_value(with_cve_option);


    with_bz_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));
    auto with_bz = parser.add_new_named_arg("with_bz");
    with_bz->set_long_name("with_bz");
    with_bz->set_short_description("show only advisories with bugzilla reference");
    with_bz->set_const_value("false");
    with_bz->link_value(with_bz_option);

    cmd.set_short_description("Manage advisories");
    cmd.set_parse_hook_func([this, &ctx](
                               [[maybe_unused]] ArgumentParser::Argument * arg,
                               [[maybe_unused]] const char * option,
                               [[maybe_unused]] int argc,
                               [[maybe_unused]] const char * const argv[]) {
        ctx.set_selected_command(this);
        return true;
    });

    cmd.register_named_arg(all);
    cmd.register_named_arg(available);
    cmd.register_named_arg(installed);
    cmd.register_named_arg(updates);

    cmd.register_named_arg(summary);
    cmd.register_named_arg(list);
    cmd.register_named_arg(info);

    cmd.register_named_arg(with_cve);
    cmd.register_named_arg(with_bz);

    cmd.register_positional_arg(specs);
}


void AdvisoryCommand::run() {
    auto & ctx = static_cast<Context &>(get_session());

    std::vector<std::string> patterns_to_show;
    if (patterns_to_show_options->size() > 0) {
        patterns_to_show.reserve(patterns_to_show_options->size());
        for (auto & pattern : *patterns_to_show_options) {
            auto option = dynamic_cast<libdnf::OptionString *>(pattern.get());
            patterns_to_show.emplace_back(option->get_value());
        }
    }

    auto package_sack = ctx.base.get_rpm_package_sack();
    package_sack->create_system_repo(false);
    libdnf::repo::RepoQuery enabled_repos(ctx.base);
    enabled_repos.filter_enabled(true);

    using LoadFlags = libdnf::rpm::PackageSack::LoadRepoFlags;
    ctx.load_rpm_repos(enabled_repos, LoadFlags::USE_UPDATEINFO);

    libdnf::rpm::PackageQuery package_query(ctx.base);
    using QueryCmp = libdnf::sack::QueryCmp;
    if (patterns_to_show.size() > 0) {
        package_query.filter_name(patterns_to_show, QueryCmp::IGLOB);
    }

    //DATA IS PREPARED

    //TODO(amatej): create advisory_query with filters on advisories prensent (if we want to limit by severity, reference..)
    auto advisory_query = libdnf::advisory::AdvisoryQuery(ctx.base);
    if (with_cve_option->get_value()) {
        advisory_query.filter_CVE("*", QueryCmp::IGLOB);
    }
    if (with_bz_option->get_value()) {
        advisory_query.filter_bug("*", QueryCmp::IGLOB);
    }
    //advisory_query.filter_name(QueryCmp::IGLOB, input_name);
    //advisory_query.filter_severity(QueryCmp::EQ, input_severity);

    std::vector<libdnf::advisory::AdvisoryPackage> result_pkgs;

    if (availability_option->get_value() == "installed") {
        auto installed_package_query = package_query.filter_installed();
        result_pkgs = advisory_query.get_advisory_packages(installed_package_query, QueryCmp::LTE);
    } else if (availability_option->get_value() == "available") {
        //TODO(amatej): filter for latest and add kernel..
        auto installed_package_query = package_query.filter_installed();
        result_pkgs = advisory_query.get_advisory_packages(installed_package_query, QueryCmp::GT);
    } else if (availability_option->get_value() == "all") {
        auto installed_package_query = package_query.filter_installed();
        result_pkgs =
            advisory_query.get_advisory_packages(installed_package_query, QueryCmp::LT | QueryCmp::EQ | QueryCmp::GT);
    } else if (availability_option->get_value() == "updates") {
        //auto upgradable_package_query = package_query.filter_upgradable().filter_nevra(keys, QueryCmp::GT);
        //result_pkgs = advisory_query.get_advisory_packages(upgradable_package_query, QueryCmp::LT | QueryCmp::EQ | QueryCmp::GT);
    }

    //TODO(amatej): output code move to libdnf-cli
    for (auto pkg : result_pkgs) {
        //auto adv = advisories_map.find(pkg.get_name() + "-" + pkg.get_arch());
        //if (adv == advisories_map.end()) {
        //    //std::cout << pkg.get_name() << std::endl << std::flush;
        std::cout
            << libdnf::advisory::Advisory(ctx.base, libdnf::advisory::AdvisoryId(pkg.get_advisory_id())).get_name()
            << " - " << pkg.get_name() << "-" << pkg.get_evr() << "." << pkg.get_arch() << std::endl;
        //} else {
        //    //std::cout << pkg.get_nevra() << " : " << advisories_map.find(pkg.get_name() + "-" + pkg.get_arch())->second.get()->get_name() << std::endl;
        //    std::cout << pkg.get_nevra() << std::endl;
        //   // std::cout << adv->first << std::endl;
        //}
    }
}


}  // namespace microdnf
