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

#include "libdnf-cli/output/repoquery.hpp"

#include <libdnf/advisory/advisory_query.hpp>
#include <libdnf/conf/const.hpp>
#include <libdnf/conf/option_string.hpp>
#include <libdnf/rpm/package.hpp>
#include <libdnf/rpm/package_query.hpp>
#include <libdnf/rpm/package_set.hpp>

#include <iostream>

namespace dnf5 {

using namespace libdnf::cli;

void RepoqueryCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("query_commands").register_argument(arg_parser_this_cmd);
}

void RepoqueryCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Search for packages matching various criteria");

    available_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(true))));

    installed_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    userinstalled_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    leaves_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    info_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    // The default format is full_nevra
    query_format_option = dynamic_cast<libdnf::OptionString *>(
        parser.add_init_value(std::make_unique<libdnf::OptionString>("%{full_nevra}\n")));

    latest_limit_option = dynamic_cast<libdnf::OptionNumber<std::int32_t> *>(
        parser.add_init_value(std::make_unique<libdnf::OptionNumber<std::int32_t>>(0)));

    auto available = parser.add_new_named_arg("available");
    available->set_long_name("available");
    available->set_description("Limit to available packages (default).");
    available->set_const_value("true");
    available->link_value(available_option);

    auto installed = parser.add_new_named_arg("installed");
    installed->set_long_name("installed");
    installed->set_description("Limit to installed packages.");
    installed->set_const_value("true");
    installed->link_value(installed_option);

    auto userinstalled = parser.add_new_named_arg("userinstalled");
    userinstalled->set_long_name("userinstalled");
    userinstalled->set_description("Limit to packages that are not installed as dependencies or weak dependencies.");
    userinstalled->set_const_value("true");
    userinstalled->link_value(userinstalled_option);
    userinstalled->add_conflict_argument(*installed);

    auto leaves = parser.add_new_named_arg("leaves");
    leaves->set_long_name("leaves");
    leaves->set_description("Limit to groups of installed packages not required by other installed packages.");
    leaves->set_const_value("true");
    leaves->link_value(leaves_option);
    leaves->add_conflict_argument(*available);

    auto info = parser.add_new_named_arg("info");
    info->set_long_name("info");
    info->set_description("Show detailed information about the packages.");
    info->set_const_value("true");
    info->link_value(info_option);

    auto query_format = parser.add_new_named_arg("queryformat");
    query_format->set_long_name("queryformat");
    query_format->set_description("Display format for packages. Default is \"%{full_nevra}\".");
    query_format->set_has_value(true);
    query_format->set_arg_value_help("QUERYFORMAT");
    query_format->link_value(query_format_option);

    auto * latest_limit = parser.add_new_named_arg("latest-limit");
    latest_limit->set_long_name("latest-limit");
    latest_limit->set_description(
        "Limit to N latest packages for a given name.arch (or all except N latest if N is negative).");
    latest_limit->set_arg_value_help("N");
    latest_limit->set_has_value(true);
    latest_limit->link_value(latest_limit_option);

    auto keys =
        parser.add_new_positional_arg("keys_to_match", ArgumentParser::PositionalArg::UNLIMITED, nullptr, nullptr);
    keys->set_description("List of keys to match");
    keys->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                pkg_specs.emplace_back(argv[i]);
            }
            return true;
        });
    keys->set_complete_hook_func([this, &ctx](const char * arg) {
        if (this->installed_option->get_value()) {
            return match_specs(ctx, arg, true, false, false, true);
        } else {
            return match_specs(ctx, arg, false, true, true, false);
        }
    });

    whatdepends_option = dynamic_cast<libdnf::OptionStringList *>(
        parser.add_init_value(std::make_unique<libdnf::OptionStringList>(std::vector<std::string>(), "", false, ",")));
    auto * whatdepends = parser.add_new_named_arg("whatdepends");
    whatdepends->set_long_name("whatdepends");
    whatdepends->set_description(
        "Limit to packages that require, enhance, recommend, suggest or supplement any of <capabilities>.");
    whatdepends->set_has_value(true);
    whatdepends->link_value(whatdepends_option);
    whatdepends->set_arg_value_help("CAPABILITY,...");

    whatconflicts_option = dynamic_cast<libdnf::OptionStringList *>(
        parser.add_init_value(std::make_unique<libdnf::OptionStringList>(std::vector<std::string>(), "", false, ",")));
    auto * whatconflicts = parser.add_new_named_arg("whatconflicts");
    whatconflicts->set_long_name("whatconflicts");
    whatconflicts->set_description("Limit to packages that conflict with any of <capabilities>.");
    whatconflicts->set_has_value(true);
    whatconflicts->link_value(whatconflicts_option);
    whatconflicts->set_arg_value_help("CAPABILITY,...");

    whatprovides_option = dynamic_cast<libdnf::OptionStringList *>(
        parser.add_init_value(std::make_unique<libdnf::OptionStringList>(std::vector<std::string>(), "", false, ",")));
    auto * whatprovides = parser.add_new_named_arg("whatprovides");
    whatprovides->set_long_name("whatprovides");
    whatprovides->set_description("Limit to packages that provide any of <capabilities>.");
    whatprovides->set_has_value(true);
    whatprovides->link_value(whatprovides_option);
    whatprovides->set_arg_value_help("CAPABILITY,...");

    whatrequires_option = dynamic_cast<libdnf::OptionStringList *>(
        parser.add_init_value(std::make_unique<libdnf::OptionStringList>(std::vector<std::string>(), "", false, ",")));
    auto * whatrequires = parser.add_new_named_arg("whatrequires");
    whatrequires->set_long_name("whatrequires");
    whatrequires->set_description(
        "Limit to packages that require any of <capabilities>. Use --whatdepends if you want to "
        "list all depending packages.");
    whatrequires->set_has_value(true);
    whatrequires->link_value(whatrequires_option);
    whatrequires->set_arg_value_help("CAPABILITY,...");

    whatobsoletes_option = dynamic_cast<libdnf::OptionStringList *>(
        parser.add_init_value(std::make_unique<libdnf::OptionStringList>(std::vector<std::string>(), "", false, ",")));
    auto * whatobsoletes = parser.add_new_named_arg("whatobsoletes");
    whatobsoletes->set_long_name("whatobsoletes");
    whatobsoletes->set_description("Limit to packages that obsolete any of <capabilities>.");
    whatobsoletes->set_has_value(true);
    whatobsoletes->link_value(whatobsoletes_option);
    whatobsoletes->set_arg_value_help("CAPABILITY,...");

    whatrecommends_option = dynamic_cast<libdnf::OptionStringList *>(
        parser.add_init_value(std::make_unique<libdnf::OptionStringList>(std::vector<std::string>(), "", false, ",")));
    auto * whatrecommends = parser.add_new_named_arg("whatrecommends");
    whatrecommends->set_long_name("whatrecommends");
    whatrecommends->set_description(
        "Limit to packages that recommend any of <capabilities>. Use --whatdepends if you want "
        "to list all depending packages.");
    whatrecommends->set_has_value(true);
    whatrecommends->link_value(whatrecommends_option);
    whatrecommends->set_arg_value_help("CAPABILITY,...");

    whatenhances_option = dynamic_cast<libdnf::OptionStringList *>(
        parser.add_init_value(std::make_unique<libdnf::OptionStringList>(std::vector<std::string>(), "", false, ",")));
    auto * whatenhances = parser.add_new_named_arg("whatenhances");
    whatenhances->set_long_name("whatenhances");
    whatenhances->set_description(
        "Limit to packages that enhance any of <capabilities>. Use --whatdepends if you want to "
        "list all depending packages.");
    whatenhances->set_has_value(true);
    whatenhances->link_value(whatenhances_option);
    whatenhances->set_arg_value_help("CAPABILITY,...");

    whatsupplements_option = dynamic_cast<libdnf::OptionStringList *>(
        parser.add_init_value(std::make_unique<libdnf::OptionStringList>(std::vector<std::string>(), "", false, ",")));
    auto * whatsupplements = parser.add_new_named_arg("whatsupplements");
    whatsupplements->set_long_name("whatsupplements");
    whatsupplements->set_description(
        "Limit to packages that supplement any of <capabilities>. Use --whatdepends if you "
        "want to list all depending packages.");
    whatsupplements->set_has_value(true);
    whatsupplements->link_value(whatsupplements_option);
    whatsupplements->set_arg_value_help("CAPABILITY,...");

    whatsuggests_option = dynamic_cast<libdnf::OptionStringList *>(
        parser.add_init_value(std::make_unique<libdnf::OptionStringList>(std::vector<std::string>(), "", false, ",")));
    auto * whatsuggests = parser.add_new_named_arg("whatsuggests");
    whatsuggests->set_long_name("whatsuggests");
    whatsuggests->set_description(
        "Limit to packages that suggest any of <capabilities>. Use --whatdepends if you want to "
        "list all depending packages.");
    whatsuggests->set_has_value(true);
    whatsuggests->link_value(whatsuggests_option);
    whatsuggests->set_arg_value_help("CAPABILITY,...");

    exactdeps = std::make_unique<libdnf::cli::session::BoolOption>(
        *this,
        "exactdeps",
        '\0',
        "Limit to packages that require <capability> specified by --whatrequires. This option is stackable "
        "with --whatrequires or --whatdepends only.",
        false);
    duplicates = std::make_unique<libdnf::cli::session::BoolOption>(
        *this,
        "duplicates",
        '\0',
        "Limit to installed duplicate packages (i.e. more package versions for  the  same  name and "
        "architecture). Installonly packages are excluded from this set.",
        false);
    unneeded = std::make_unique<libdnf::cli::session::BoolOption>(
        *this,
        "unneeded",
        '\0',
        "Limit to unneeded installed packages (i.e. packages that were installed as "
        "dependencies but are no longer needed).",
        false);

    advisory_name = std::make_unique<AdvisoryOption>(*this);
    advisory_security = std::make_unique<SecurityOption>(*this);
    advisory_bugfix = std::make_unique<BugfixOption>(*this);
    advisory_enhancement = std::make_unique<EnhancementOption>(*this);
    advisory_newpackage = std::make_unique<NewpackageOption>(*this);
    advisory_severity = std::make_unique<AdvisorySeverityOption>(*this);
    advisory_bz = std::make_unique<BzOption>(*this);
    advisory_cve = std::make_unique<CveOption>(*this);

    std::vector<std::string> pkg_attrs_options{
        "conflicts",
        "enhances",
        "obsoletes",
        "provides",
        "recommends",
        "requires",
        "requires-pre",
        "suggests",
        "supplements",
        "",  // empty when option is not used
    };
    pkg_attr_option = dynamic_cast<libdnf::OptionEnum<std::string> *>(
        parser.add_init_value(std::make_unique<libdnf::OptionEnum<std::string>>("", pkg_attrs_options)));

    auto * repoquery_formatting = get_context().get_argument_parser().add_new_group("repoquery_formatting");
    repoquery_formatting->set_header("Formatting:");
    cmd.register_group(repoquery_formatting);

    // remove the last empty ("") option, it should not be an arg
    pkg_attrs_options.pop_back();
    for (const auto & pkg_attr : pkg_attrs_options) {
        auto * arg = parser.add_new_named_arg(pkg_attr);
        arg->set_long_name(pkg_attr);
        arg->set_description("Like --queryformat=\"%{" + pkg_attr + "}\" but deduplicated and sorted.");
        arg->set_has_value(false);
        arg->set_const_value(pkg_attr);
        arg->link_value(pkg_attr_option);
        repoquery_formatting->register_argument(arg);
        cmd.register_named_arg(arg);
    }
    repoquery_formatting->register_argument(info);
    cmd.register_named_arg(info);
    repoquery_formatting->register_argument(query_format);
    cmd.register_named_arg(query_format);

    cmd.register_named_arg(available);
    cmd.register_named_arg(installed);
    cmd.register_named_arg(userinstalled);
    cmd.register_named_arg(leaves);
    cmd.register_named_arg(latest_limit);

    cmd.register_named_arg(whatdepends);
    cmd.register_named_arg(whatconflicts);
    cmd.register_named_arg(whatprovides);
    cmd.register_named_arg(whatrequires);
    cmd.register_named_arg(whatobsoletes);
    cmd.register_named_arg(whatrecommends);
    cmd.register_named_arg(whatenhances);
    cmd.register_named_arg(whatsupplements);
    cmd.register_named_arg(whatsuggests);


    cmd.register_positional_arg(keys);
}

void RepoqueryCommand::configure() {
    auto & context = get_context();
    context.update_repo_metadata_from_specs(pkg_specs);
    only_system_repo_needed = installed_option->get_value() || userinstalled_option->get_value() ||
                              duplicates->get_value() || leaves_option->get_value() || unneeded->get_value();
    context.set_load_system_repo(only_system_repo_needed);
    bool updateinfo_needed = advisory_name->get_value().empty() || advisory_security->get_value() ||
                             advisory_bugfix->get_value() || advisory_enhancement->get_value() ||
                             advisory_newpackage->get_value() || advisory_severity->get_value().empty() ||
                             advisory_bz->get_value().empty() || advisory_cve->get_value().empty();
    if (updateinfo_needed) {
        context.base.get_config().get_optional_metadata_types_option().add_item(libdnf::METADATA_TYPE_UPDATEINFO);
    }
    context.set_load_available_repos(
        available_option->get_priority() >= libdnf::Option::Priority::COMMANDLINE || !only_system_repo_needed
            ? Context::LoadAvailableRepos::ENABLED
            : Context::LoadAvailableRepos::NONE);
}

void RepoqueryCommand::load_additional_packages() {
    if (available_option->get_priority() >= libdnf::Option::Priority::COMMANDLINE || !only_system_repo_needed) {
        auto & ctx = get_context();
        for (auto & [path, package] : ctx.base.get_repo_sack()->add_cmdline_packages(pkg_specs)) {
            cmdline_packages.push_back(std::move(package));
        }
    }
}

// In case of input being nevras -> resolve them to packages
static libdnf::rpm::PackageSet resolve_nevras_to_packges(
    libdnf::Base & base, const std::vector<std::string> & nevra_globs, const libdnf::rpm::PackageQuery & base_query) {
    auto resolved_nevras_set = libdnf::rpm::PackageSet(base);
    auto settings = libdnf::ResolveSpecSettings();
    settings.with_provides = false;
    settings.with_filenames = false;
    for (const auto & nevra : nevra_globs) {
        auto tmp_query = base_query;
        tmp_query.resolve_pkg_spec(nevra, settings, true);
        resolved_nevras_set |= tmp_query;
    }

    return resolved_nevras_set;
}

void RepoqueryCommand::run() {
    auto & ctx = get_context();

    libdnf::rpm::PackageSet result_pset(ctx.base);
    libdnf::rpm::PackageQuery full_package_query(ctx.base);

    if (leaves_option->get_value()) {
        full_package_query.filter_leaves();
    }

    if (userinstalled_option->get_value()) {
        full_package_query.filter_userinstalled();
    }

    auto advisories = advisory_query_from_cli_input(
        ctx.base,
        advisory_name->get_value(),
        advisory_security->get_value(),
        advisory_bugfix->get_value(),
        advisory_enhancement->get_value(),
        advisory_newpackage->get_value(),
        advisory_severity->get_value(),
        advisory_bz->get_value(),
        advisory_cve->get_value());
    if (advisories.has_value()) {
        full_package_query.filter_advisories(advisories.value(), libdnf::sack::QueryCmp::GTE);
    }

    if (latest_limit_option->get_value() != 0) {
        full_package_query.filter_latest_evr(latest_limit_option->get_value());
    }

    if (!whatdepends_option->get_value().empty()) {
        auto matched_reldeps = libdnf::rpm::ReldepList(ctx.base);
        for (const auto & reldep_glob : whatdepends_option->get_value()) {
            matched_reldeps.add_reldep_with_glob(reldep_glob);
        }

        // Filter requires by reldeps
        auto dependsquery = full_package_query;
        dependsquery.filter_requires(matched_reldeps, libdnf::sack::QueryCmp::EQ);

        // Filter weak deps via reldeps
        auto recommends_reldep_query = full_package_query;
        recommends_reldep_query.filter_recommends(matched_reldeps, libdnf::sack::QueryCmp::EQ);
        dependsquery |= recommends_reldep_query;
        auto enhances_reldep_query = full_package_query;
        enhances_reldep_query.filter_enhances(matched_reldeps, libdnf::sack::QueryCmp::EQ);
        dependsquery |= enhances_reldep_query;
        auto supplements_reldep_query = full_package_query;
        supplements_reldep_query.filter_supplements(matched_reldeps, libdnf::sack::QueryCmp::EQ);
        dependsquery |= supplements_reldep_query;
        auto suggests_reldep_query = full_package_query;
        suggests_reldep_query.filter_suggests(matched_reldeps, libdnf::sack::QueryCmp::EQ);
        dependsquery |= suggests_reldep_query;

        if (!exactdeps->get_value()) {
            auto pkgs_from_resolved_nevras =
                resolve_nevras_to_packges(ctx.base, whatdepends_option->get_value(), full_package_query);

            // Filter requires by packages from resolved nevras
            auto what_requires_resolved_nevras = full_package_query;
            what_requires_resolved_nevras.filter_requires(pkgs_from_resolved_nevras);
            dependsquery |= what_requires_resolved_nevras;

            // Filter weak deps by packages from resolved nevras
            auto recommends_pkg_query = full_package_query;
            recommends_pkg_query.filter_recommends(pkgs_from_resolved_nevras, libdnf::sack::QueryCmp::EQ);
            dependsquery |= recommends_pkg_query;
            auto enhances_pkg_query = full_package_query;
            enhances_pkg_query.filter_enhances(pkgs_from_resolved_nevras, libdnf::sack::QueryCmp::EQ);
            dependsquery |= enhances_pkg_query;
            auto supplements_pkg_query = full_package_query;
            supplements_pkg_query.filter_supplements(pkgs_from_resolved_nevras, libdnf::sack::QueryCmp::EQ);
            dependsquery |= supplements_pkg_query;
            auto suggests_pkg_query = full_package_query;
            suggests_pkg_query.filter_suggests(pkgs_from_resolved_nevras, libdnf::sack::QueryCmp::EQ);
            dependsquery |= suggests_pkg_query;
        }
        //TODO(amatej): add recurisve option call

        full_package_query = dependsquery;
    }
    if (!whatprovides_option->get_value().empty()) {
        auto provides_query = full_package_query;
        provides_query.filter_provides(whatprovides_option->get_value(), libdnf::sack::QueryCmp::GLOB);
        if (!provides_query.empty()) {
            full_package_query = provides_query;
        } else {
            // If provides query doesn't match anything try matching files
            full_package_query.filter_file(whatprovides_option->get_value(), libdnf::sack::QueryCmp::GLOB);
        }
    }
    if (!whatrequires_option->get_value().empty()) {
        if (exactdeps->get_value()) {
            full_package_query.filter_requires(whatrequires_option->get_value(), libdnf::sack::QueryCmp::GLOB);
        } else {
            auto requires_resolved = full_package_query;
            requires_resolved.filter_requires(
                resolve_nevras_to_packges(ctx.base, whatrequires_option->get_value(), full_package_query));

            full_package_query.filter_requires(whatrequires_option->get_value(), libdnf::sack::QueryCmp::GLOB);
            full_package_query |= requires_resolved;
            //TODO(amatej): add recurisve option call
        }
    }
    if (!whatobsoletes_option->get_value().empty()) {
        full_package_query.filter_obsoletes(whatobsoletes_option->get_value(), libdnf::sack::QueryCmp::GLOB);
    }
    if (!whatconflicts_option->get_value().empty()) {
        auto conflicts_resolved = full_package_query;
        conflicts_resolved.filter_conflicts(
            resolve_nevras_to_packges(ctx.base, whatconflicts_option->get_value(), full_package_query));

        full_package_query.filter_conflicts(whatconflicts_option->get_value(), libdnf::sack::QueryCmp::GLOB);
        full_package_query |= conflicts_resolved;
    }
    if (!whatrecommends_option->get_value().empty()) {
        auto recommends_resolved = full_package_query;
        recommends_resolved.filter_recommends(
            resolve_nevras_to_packges(ctx.base, whatrecommends_option->get_value(), full_package_query));

        full_package_query.filter_recommends(whatrecommends_option->get_value(), libdnf::sack::QueryCmp::GLOB);
        full_package_query |= recommends_resolved;
    }
    if (!whatenhances_option->get_value().empty()) {
        auto enhances_resolved = full_package_query;
        enhances_resolved.filter_enhances(
            resolve_nevras_to_packges(ctx.base, whatenhances_option->get_value(), full_package_query));

        full_package_query.filter_enhances(whatenhances_option->get_value(), libdnf::sack::QueryCmp::GLOB);
        full_package_query |= enhances_resolved;
    }
    if (!whatsupplements_option->get_value().empty()) {
        auto supplements_resolved = full_package_query;
        supplements_resolved.filter_supplements(
            resolve_nevras_to_packges(ctx.base, whatsupplements_option->get_value(), full_package_query));

        full_package_query.filter_supplements(whatsupplements_option->get_value(), libdnf::sack::QueryCmp::GLOB);
        full_package_query |= supplements_resolved;
    }
    if (!whatsuggests_option->get_value().empty()) {
        auto suggests_resolved = full_package_query;
        suggests_resolved.filter_suggests(
            resolve_nevras_to_packges(ctx.base, whatsuggests_option->get_value(), full_package_query));

        full_package_query.filter_suggests(whatsuggests_option->get_value(), libdnf::sack::QueryCmp::GLOB);
        full_package_query |= suggests_resolved;
    }

    if (duplicates->get_value()) {
        auto & cfg_main = ctx.base.get_config();
        const auto & installonly_packages = cfg_main.get_installonlypkgs_option().get_value();
        auto installonly_query = full_package_query;
        installonly_query.filter_provides(installonly_packages, libdnf::sack::QueryCmp::GLOB);
        full_package_query -= installonly_query;
        full_package_query.filter_duplicates();
    }

    if (unneeded->get_value()) {
        full_package_query.filter_unneeded();
    }

    if (pkg_specs.empty()) {
        result_pset |= full_package_query;
    } else {
        for (const auto & pkg : cmdline_packages) {
            if (full_package_query.contains(pkg)) {
                result_pset.add(pkg);
            }
        }

        const libdnf::ResolveSpecSettings settings{.ignore_case = true, .with_provides = false};
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
    } else if (!pkg_attr_option->get_value().empty()) {
        libdnf::cli::output::print_pkg_attr_uniq_sorted(stdout, result_pset, pkg_attr_option->get_value());
    } else {
        libdnf::cli::output::print_pkg_set_with_format(stdout, result_pset, query_format_option->get_value());
    }
}

}  // namespace dnf5
