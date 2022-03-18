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


#include "repoquery.hpp"

#include "context.hpp"

#include "libdnf-cli/output/repoquery.hpp"

#include <libdnf/conf/option_string.hpp>
#include <libdnf/rpm/package.hpp>
#include <libdnf/rpm/package_query.hpp>
#include <libdnf/rpm/package_set.hpp>

#include <iostream>


namespace microdnf {


using namespace libdnf::cli;


RepoqueryCommand::RepoqueryCommand(Command & parent) : Command(parent, "repoquery") {
    auto & ctx = static_cast<Context &>(get_session());
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("Search for packages matching various criteria");

    available_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(true))));

    installed_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    info_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    nevra_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(true))));

    auto available = parser.add_new_named_arg("available");
    available->set_long_name("available");
    available->set_short_description("display available packages (default)");
    available->set_const_value("true");
    available->link_value(available_option);

    auto installed = parser.add_new_named_arg("installed");
    installed->set_long_name("installed");
    installed->set_short_description("display installed packages");
    installed->set_const_value("true");
    installed->link_value(installed_option);

    auto info = parser.add_new_named_arg("info");
    info->set_long_name("info");
    info->set_short_description("show detailed information about the packages");
    info->set_const_value("true");
    info->link_value(info_option);

    auto nevra = parser.add_new_named_arg("nevra");
    nevra->set_long_name("nevra");
    nevra->set_short_description(
        "use name-epoch:version-release.architecture format for displaying packages (default)");
    nevra->set_const_value("true");
    nevra->link_value(nevra_option);

    auto keys =
        parser.add_new_positional_arg("keys_to_match", ArgumentParser::PositionalArg::UNLIMITED, nullptr, nullptr);
    keys->set_short_description("List of keys to match");
    keys->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            parse_add_specs(argc, argv, pkg_specs, pkg_file_paths);
            return true;
        });
    keys->set_complete_hook_func([this, &ctx](const char * arg) {
        if (this->installed_option->get_value()) {
            return match_installed_pkgs(ctx, arg, true);
        } else {
            return match_available_pkgs(ctx, arg);
        }
    });

    auto conflict_args = parser.add_conflict_args_group(std::unique_ptr<std::vector<ArgumentParser::Argument *>>(
        new std::vector<ArgumentParser::Argument *>{info, nevra}));

    info->set_conflict_arguments(conflict_args);
    nevra->set_conflict_arguments(conflict_args);

    cmd.register_named_arg(available);
    cmd.register_named_arg(installed);
    cmd.register_named_arg(info);
    cmd.register_named_arg(nevra);
    cmd.register_positional_arg(keys);
}


void RepoqueryCommand::run() {
    auto & ctx = static_cast<Context &>(get_session());
    std::vector<libdnf::rpm::Package> cmdline_packages;

    if (installed_option->get_value()) {
        ctx.base.get_repo_sack()->get_system_repo()->load();
    }

    if (available_option->get_priority() >= libdnf::Option::Priority::COMMANDLINE || !installed_option->get_value()) {
        // load available repositories
        ctx.load_repos(false);

        std::vector<std::string> error_messages;
        cmdline_packages = ctx.add_cmdline_packages(pkg_file_paths, error_messages);
        for (const auto & msg : error_messages) {
            std::cout << msg << std::endl;
        }
    } else {
        // TODO(lukash) this is inconvenient, we should try to call it automatically at the right time in libdnf
        ctx.base.get_rpm_package_sack()->setup_excludes_includes();
    }

    libdnf::rpm::PackageSet result_pset(ctx.base);

    for (const auto & pkg : cmdline_packages) {
        if (!pkg.is_excluded()) {
            result_pset.add(pkg);
        }
    }
    if (!pkg_specs.empty()) {
        const libdnf::ResolveSpecSettings settings{.ignore_case = true, .with_provides = false};
        const libdnf::rpm::PackageQuery full_package_query(ctx.base);
        for (const auto & spec : pkg_specs) {
            libdnf::rpm::PackageQuery package_query(full_package_query);
            package_query.resolve_pkg_spec(spec, settings, true);
            result_pset |= package_query;
        }
    }

    if (info_option->get_value()) {
        for (auto package : result_pset) {
            libdnf::cli::output::print_package_info_table(package);
            std::cout << '\n';
        }
    } else {
        for (auto package : result_pset) {
            std::cout << package.get_full_nevra() << '\n';
        }
    }
}


}  // namespace microdnf
