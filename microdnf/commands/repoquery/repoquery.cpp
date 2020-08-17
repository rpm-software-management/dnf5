/*
Copyright (C) 2019-2020 Red Hat, Inc.

This file is part of microdnf: https://github.com/rpm-software-management/libdnf/

Microdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Microdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with microdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "repoquery.hpp"

#include "../../context.hpp"

#include <libdnf/conf/option_string.hpp>
#include <libdnf/rpm/package.hpp>
#include <libdnf/rpm/package_set.hpp>
#include <libdnf/rpm/solv_query.hpp>
#include <libsmartcols/libsmartcols.h>

#include <iostream>

namespace microdnf {

static void package_info_add_line(struct libscols_table * table, const char * key, const char * value) {
    struct libscols_line * ln = scols_table_new_line(table, nullptr);
    scols_line_set_data(ln, 0, key);
    scols_line_set_data(ln, 1, value);
}

static void print_package_info(libdnf::rpm::Package & package) {
    struct libscols_table * table = scols_new_table();
    scols_table_enable_noheadings(table, 1);
    scols_table_set_column_separator(table, " : ");
    scols_table_new_column(table, "key", 5, 0);
    struct libscols_column * cl = scols_table_new_column(table, "value", 10, SCOLS_FL_WRAP);
    scols_column_set_safechars(cl, "\n");
    scols_column_set_wrapfunc(cl, scols_wrapnl_chunksize, scols_wrapnl_nextchunk, nullptr);

    package_info_add_line(table, "Name", package.get_name().c_str());
    auto epoch = package.get_epoch();
    if (epoch != 0) {
        auto str_epoch = std::to_string(epoch);
        package_info_add_line(table, "Epoch", str_epoch.c_str());
    }
    package_info_add_line(table, "Version", package.get_version().c_str());
    package_info_add_line(table, "Release", package.get_release().c_str());
    package_info_add_line(table, "Architecture", package.get_arch().c_str());
    auto size = package.get_size();
    package_info_add_line(table, "Size", std::to_string(size).c_str());
    package_info_add_line(table, "Source", package.get_sourcerpm().c_str());
    // TODO(jrohel): support for reponame package_info_add_line(table, "Repository", package.get_reponame().c_str());
    package_info_add_line(table, "Summary", package.get_summary().c_str());
    package_info_add_line(table, "URL", package.get_url().c_str());
    package_info_add_line(table, "License", package.get_license().c_str());
    package_info_add_line(table, "Description", package.get_description().c_str());

    scols_print_table(table);
    scols_unref_table(table);
}

void CmdRepoquery::set_argument_parser(Context & ctx) {
    available_option = dynamic_cast<libdnf::OptionBool *>(
        ctx.arg_parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(true))));

    installed_option = dynamic_cast<libdnf::OptionBool *>(
        ctx.arg_parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    info_option = dynamic_cast<libdnf::OptionBool *>(
        ctx.arg_parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    nevra_option = dynamic_cast<libdnf::OptionBool *>(
        ctx.arg_parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(true))));

    auto available = ctx.arg_parser.add_new_named_arg("available");
    available->set_long_name("available");
    available->set_short_description("display available packages (default)");
    available->set_const_value("true");
    available->link_value(available_option);

    auto installed = ctx.arg_parser.add_new_named_arg("installed");
    installed->set_long_name("installed");
    installed->set_short_description("display installed packages");
    installed->set_const_value("true");
    installed->link_value(installed_option);

    auto info = ctx.arg_parser.add_new_named_arg("info");
    info->set_long_name("info");
    info->set_short_description("show detailed information about the packages");
    info->set_const_value("true");
    info->link_value(info_option);

    auto nevra = ctx.arg_parser.add_new_named_arg("nevra");
    nevra->set_long_name("nevra");
    nevra->set_short_description(
        "use name-epoch:version-release.architecture format for displaying packages (default)");
    nevra->set_const_value("true");
    nevra->link_value(nevra_option);

    patterns_to_show_options = ctx.arg_parser.add_new_values();
    auto keys = ctx.arg_parser.add_new_positional_arg(
        "keys_to_match",
        ArgumentParser::PositionalArg::UNLIMITED,
        ctx.arg_parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_to_show_options);
    keys->set_short_description("List of keys to match");

    auto conflict_args =
        ctx.arg_parser.add_conflict_args_group(std::unique_ptr<std::vector<ArgumentParser::Argument *>>(
            new std::vector<ArgumentParser::Argument *>{info, nevra}));

    info->set_conflict_arguments(conflict_args);
    nevra->set_conflict_arguments(conflict_args);

    auto repoquery = ctx.arg_parser.add_new_command("repoquery");
    repoquery->set_short_description("search for packages matching keyword");
    repoquery->set_description("");
    repoquery->named_args_help_header = "Optional arguments:";
    repoquery->positional_args_help_header = "Positional arguments:";
    repoquery->parse_hook = [this, &ctx](
                                [[maybe_unused]] ArgumentParser::Argument * arg,
                                [[maybe_unused]] const char * option,
                                [[maybe_unused]] int argc,
                                [[maybe_unused]] const char * const argv[]) {
        ctx.select_command(this);
        return true;
    };

    repoquery->add_named_arg(available);
    repoquery->add_named_arg(installed);
    repoquery->add_named_arg(info);
    repoquery->add_named_arg(nevra);
    repoquery->add_positional_arg(keys);

    ctx.arg_parser.get_root_command()->add_command(repoquery);
}

void CmdRepoquery::configure([[maybe_unused]] Context & ctx) {}

void CmdRepoquery::run(Context & ctx) {
    using LoadFlags = libdnf::rpm::SolvSack::LoadRepoFlags;
    auto & solv_sack = ctx.base.get_rpm_solv_sack();

    // To search in the system repository (installed packages)
    if (installed_option->get_value()) {
        // Creates system repository in the repo_sack and loads it into rpm::SolvSack.
        solv_sack.create_system_repo(false);
    }

    // To search in available repositories (available packages)
    if (available_option->get_priority() >= libdnf::Option::Priority::COMMANDLINE || !installed_option->get_value()) {
        auto enabled_repos = ctx.base.get_rpm_repo_sack().new_query().ifilter_enabled(true);
        for (auto & repo : enabled_repos.get_data()) {
            ctx.load_rpm_repo(*repo.get());
        }

        for (auto & repo : enabled_repos.get_data()) {
            solv_sack.load_repo(
                *repo.get(),
                LoadFlags::USE_FILELISTS | LoadFlags::USE_PRESTO | LoadFlags::USE_UPDATEINFO | LoadFlags::USE_OTHER);
        }
    }

    libdnf::rpm::PackageSet result_pset(&solv_sack);
    libdnf::rpm::SolvQuery full_solv_query(&solv_sack);
    for (auto & pattern : *patterns_to_show_options) {
        libdnf::rpm::SolvQuery solv_query(full_solv_query);
        solv_query.resolve_pkg_spec(dynamic_cast<libdnf::OptionString *>(pattern.get())->get_value(), true, true, true, true, true, {});
        result_pset |= solv_query.get_package_set();
    }

    if (info_option->get_value()) {
        for (auto package : result_pset) {
            print_package_info(package);
            std::cout << '\n';
        }
    } else {
        for (auto package : result_pset) {
            std::cout << package.get_full_nevra() << '\n';
        }
    }
}

}  // namespace microdnf
