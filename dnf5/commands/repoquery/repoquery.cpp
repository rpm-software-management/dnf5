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

#include "libdnf5-cli/output/changelogs.hpp"
#include "libdnf5-cli/output/repoquery.hpp"

#include "libdnf5/utils/patterns.hpp"

#include <dnf5/shared_options.hpp>
#include <libdnf5/advisory/advisory_query.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/conf/option_string.hpp>
#include <libdnf5/rpm/package.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/package_set.hpp>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

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

    available_option = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionBool>(new libdnf5::OptionBool(true))));

    installed_option = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionBool>(new libdnf5::OptionBool(false))));

    userinstalled_option = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionBool>(new libdnf5::OptionBool(false))));

    leaves_option = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionBool>(new libdnf5::OptionBool(false))));

    info_option = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionBool>(new libdnf5::OptionBool(false))));

    // The default format is full_nevra
    query_format_option = dynamic_cast<libdnf5::OptionString *>(
        parser.add_init_value(std::make_unique<libdnf5::OptionString>("%{full_nevra}\n")));

    latest_limit_option = dynamic_cast<libdnf5::OptionNumber<std::int32_t> *>(
        parser.add_init_value(std::make_unique<libdnf5::OptionNumber<std::int32_t>>(0)));

    querytags_option =
        dynamic_cast<libdnf5::OptionBool *>(parser.add_init_value(std::make_unique<libdnf5::OptionBool>(false)));


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

    auto query_tags = parser.add_new_named_arg("querytags");
    query_tags->set_long_name("querytags");
    query_tags->set_description("Display available tags for --queryformat.");
    query_tags->set_const_value("true");
    query_tags->link_value(querytags_option);

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

    whatdepends = std::make_unique<libdnf5::cli::session::AppendStringListOption>(
        *this,
        "whatdepends",
        '\0',
        "Limit to packages that require, enhance, recommend, suggest or supplement any of <capabilities>.",
        "CAPABILITY,...",
        "",
        false,
        ",");
    whatconflicts = std::make_unique<libdnf5::cli::session::AppendStringListOption>(
        *this,
        "whatconflicts",
        '\0',
        "Limit to packages that conflict with any of <capabilities>.",
        "CAPABILITY,...",
        "",
        false,
        ",");
    whatenhances = std::make_unique<libdnf5::cli::session::AppendStringListOption>(
        *this,
        "whatenhances",
        '\0',
        "Limit to packages that enhance any of <capabilities>. Use --whatdepends if you want to "
        "list all depending packages.",
        "CAPABILITY,...",
        "",
        false,
        ",");
    whatobsoletes = std::make_unique<libdnf5::cli::session::AppendStringListOption>(
        *this,
        "whatobsoletes",
        '\0',
        "Limit to packages that obsolete any of <capabilities>.",
        "CAPABILITY,...",
        "",
        false,
        ",");
    whatprovides = std::make_unique<libdnf5::cli::session::AppendStringListOption>(
        *this,
        "whatprovides",
        '\0',
        "Limit to packages that provide any of <capabilities>.",
        "CAPABILITY,...",
        "",
        false,
        ",");
    whatrecommends = std::make_unique<libdnf5::cli::session::AppendStringListOption>(
        *this,
        "whatrecommends",
        '\0',
        "Limit to packages that recommend any of <capabilities>. Use --whatdepends if you want "
        "to list all depending packages.",
        "CAPABILITY,...",
        "",
        false,
        ",");
    whatrequires = std::make_unique<libdnf5::cli::session::AppendStringListOption>(
        *this,
        "whatrequires",
        '\0',
        "Limit to packages that require any of <capabilities>. Use --whatdepends if you want to "
        "list all depending packages.",
        "CAPABILITY,...",
        "",
        false,
        ",");
    whatsupplements = std::make_unique<libdnf5::cli::session::AppendStringListOption>(
        *this,
        "whatsupplements",
        '\0',
        "Limit to packages that supplement any of <capabilities>. Use --whatdepends if you "
        "want to list all depending packages.",
        "CAPABILITY,...",
        "",
        false,
        ",");
    whatsuggests = std::make_unique<libdnf5::cli::session::AppendStringListOption>(
        *this,
        "whatsuggests",
        '\0',
        "Limit to packages that suggest any of <capabilities>. Use --whatdepends if you want to "
        "list all depending packages.",
        "CAPABILITY,...",
        "",
        false,
        ",");
    create_forcearch_option(*this);
    arch = std::make_unique<libdnf5::cli::session::AppendStringListOption>(
        *this, "arch", '\0', "Limit to packages of these architectures.", "ARCH,...", "", false, ",");
    file = std::make_unique<libdnf5::cli::session::AppendStringListOption>(
        *this, "file", '\0', "Limit to packages that own these files.", "FILE,...", "", false, ",");

    exactdeps = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this,
        "exactdeps",
        '\0',
        "Limit to packages that require <capability> specified by --whatrequires. This option is stackable "
        "with --whatrequires or --whatdepends only.",
        false);
    duplicates = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this,
        "duplicates",
        '\0',
        "Limit to installed duplicate packages (i.e. more package versions for  the  same  name and "
        "architecture). Installonly packages are excluded from this set.",
        false);
    unneeded = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this,
        "unneeded",
        '\0',
        "Limit to unneeded installed packages (i.e. packages that were installed as "
        "dependencies but are no longer needed).",
        false);
    extras = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "extras", '\0', "Limit to installed packages that are not present in any available repository.", false);
    upgrades = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this,
        "upgrades",
        '\0',
        "Limit to packages that provide an upgrade for some already installed package.",
        false);
    recent = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "recent", '\0', "Limit to only recently changed packages.", false);
    installonly = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "installonly", '\0', "Limit to installed installonly packages.", false);

    advisory_name = std::make_unique<AdvisoryOption>(*this);
    advisory_security = std::make_unique<SecurityOption>(*this);
    advisory_bugfix = std::make_unique<BugfixOption>(*this);
    advisory_enhancement = std::make_unique<EnhancementOption>(*this);
    advisory_newpackage = std::make_unique<NewpackageOption>(*this);
    advisory_severity = std::make_unique<AdvisorySeverityOption>(*this);
    advisory_bz = std::make_unique<BzOption>(*this);
    advisory_cve = std::make_unique<CveOption>(*this);

    changelogs = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "changelogs", '\0', "Display package changelogs.", false);

    std::vector<std::string> pkg_attrs_options{
        "conflicts",
        "depends",
        "enhances",
        "obsoletes",
        "provides",
        "recommends",
        "requires",
        "requires_pre",
        "suggests",
        "supplements",
        "files",
        "sourcerpm",
        "",  // empty when option is not used
    };
    pkg_attr_option = dynamic_cast<libdnf5::OptionEnum<std::string> *>(
        parser.add_init_value(std::make_unique<libdnf5::OptionEnum<std::string>>("", pkg_attrs_options)));

    auto * repoquery_formatting = get_context().get_argument_parser().add_new_group("repoquery_formatting");
    repoquery_formatting->set_header("Formatting:");
    cmd.register_group(repoquery_formatting);

    // remove the last empty ("") option, it should not be an arg
    pkg_attrs_options.pop_back();
    for (auto & pkg_attr : pkg_attrs_options) {
        auto * arg = parser.add_new_named_arg(pkg_attr);
        arg->set_description("Like --queryformat=\"%{" + pkg_attr + "}\" but deduplicated and sorted.");
        arg->set_has_value(false);
        arg->set_const_value(pkg_attr);
        // The option names use '-' separator instead of '_'
        std::replace(pkg_attr.begin(), pkg_attr.end(), '_', '-');
        arg->set_long_name(pkg_attr);
        arg->link_value(pkg_attr_option);
        repoquery_formatting->register_argument(arg);
        cmd.register_named_arg(arg);
    }
    repoquery_formatting->register_argument(info);
    cmd.register_named_arg(info);
    repoquery_formatting->register_argument(query_format);
    cmd.register_named_arg(query_format);
    repoquery_formatting->register_argument(query_tags);
    cmd.register_named_arg(query_tags);
    repoquery_formatting->register_argument(changelogs->arg);

    cmd.register_named_arg(available);
    cmd.register_named_arg(installed);
    cmd.register_named_arg(userinstalled);
    cmd.register_named_arg(leaves);
    cmd.register_named_arg(latest_limit);

    cmd.register_positional_arg(keys);
}

void RepoqueryCommand::configure() {
    auto & context = get_context();
    context.update_repo_metadata_from_specs(pkg_specs);
    system_repo_needed = installed_option->get_value() || userinstalled_option->get_value() ||
                         duplicates->get_value() || leaves_option->get_value() || unneeded->get_value() ||
                         extras->get_value() || upgrades->get_value() || recent->get_value() ||
                         installonly->get_value();
    context.set_load_system_repo(system_repo_needed);
    bool updateinfo_needed = advisory_name->get_value().empty() || advisory_security->get_value() ||
                             advisory_bugfix->get_value() || advisory_enhancement->get_value() ||
                             advisory_newpackage->get_value() || advisory_severity->get_value().empty() ||
                             advisory_bz->get_value().empty() || advisory_cve->get_value().empty();
    if (updateinfo_needed) {
        context.base.get_config().get_optional_metadata_types_option().add_item(
            libdnf5::Option::Priority::RUNTIME, libdnf5::METADATA_TYPE_UPDATEINFO);
    }
    context.set_load_available_repos(
        // available_option is on by default, to check if user specified it we check priority
        available_option->get_priority() >= libdnf5::Option::Priority::COMMANDLINE || !system_repo_needed ||
                extras->get_value() || upgrades->get_value() || recent->get_value()
            ? Context::LoadAvailableRepos::ENABLED
            : Context::LoadAvailableRepos::NONE);

    if (changelogs->get_value()) {
        context.base.get_config().get_optional_metadata_types_option().add_item(
            libdnf5::Option::Priority::RUNTIME, libdnf5::METADATA_TYPE_OTHER);
    }

    if ((pkg_attr_option->get_value() == "files") ||
        (libdnf5::cli::output::requires_filelists(query_format_option->get_value()))) {
        context.base.get_config().get_optional_metadata_types_option().add_item(
            libdnf5::Option::Priority::RUNTIME, libdnf5::METADATA_TYPE_FILELISTS);
        return;
    }
    for (const auto & capabilities :
         {whatrequires->get_value(),
          whatdepends->get_value(),
          whatconflicts->get_value(),
          whatprovides->get_value(),
          whatobsoletes->get_value(),
          whatrecommends->get_value(),
          whatenhances->get_value(),
          whatsupplements->get_value(),
          whatsuggests->get_value()}) {
        for (const auto & capability : capabilities) {
            if (libdnf5::utils::is_file_pattern(capability)) {
                context.base.get_config().get_optional_metadata_types_option().add_item(
                    libdnf5::Option::Priority::RUNTIME, libdnf5::METADATA_TYPE_FILELISTS);
                return;
            }
        }
    }
}

void RepoqueryCommand::load_additional_packages() {
    auto & ctx = get_context();
    if (ctx.get_load_available_repos() != Context::LoadAvailableRepos::NONE) {
        for (auto & [path, package] : ctx.base.get_repo_sack()->add_cmdline_packages(pkg_specs)) {
            cmdline_packages.push_back(std::move(package));
        }
    }
}

// In case of input being nevras -> resolve them to packages
static libdnf5::rpm::PackageSet resolve_nevras_to_packges(
    libdnf5::Base & base, const std::vector<std::string> & nevra_globs, const libdnf5::rpm::PackageQuery & base_query) {
    auto resolved_nevras_set = libdnf5::rpm::PackageSet(base);
    auto settings = libdnf5::ResolveSpecSettings();
    settings.with_provides = false;
    settings.with_filenames = false;
    for (const auto & nevra : nevra_globs) {
        auto tmp_query = base_query;
        tmp_query.resolve_pkg_spec(nevra, settings, true);
        resolved_nevras_set |= tmp_query;
    }

    return resolved_nevras_set;
}

static libdnf5::rpm::PackageQuery get_installonly_query(libdnf5::Base & base) {
    auto & cfg_main = base.get_config();
    const auto & installonly_packages = cfg_main.get_installonlypkgs_option().get_value();
    libdnf5::rpm::PackageQuery installonly_query(base);
    installonly_query.filter_provides(installonly_packages, libdnf5::sack::QueryCmp::GLOB);
    return installonly_query;
}

void RepoqueryCommand::run() {
    auto & ctx = get_context();

    libdnf5::rpm::PackageQuery base_query(ctx.base);
    libdnf5::rpm::PackageQuery full_package_query(ctx.base, libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES, true);

    // First filter by pkg_specs - it can narrow the query the most
    if (pkg_specs.empty()) {
        full_package_query |= base_query;
    } else {
        for (const auto & pkg : cmdline_packages) {
            if (base_query.contains(pkg)) {
                full_package_query.add(pkg);
            }
        }

        const libdnf5::ResolveSpecSettings settings{.ignore_case = true, .with_provides = false};
        for (const auto & spec : pkg_specs) {
            libdnf5::rpm::PackageQuery package_query(base_query);
            package_query.resolve_pkg_spec(spec, settings, true);
            full_package_query |= package_query;
        }
    }

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
        full_package_query.filter_advisories(advisories.value(), libdnf5::sack::QueryCmp::GTE);
    }

    if (latest_limit_option->get_value() != 0) {
        full_package_query.filter_latest_evr(latest_limit_option->get_value());
    }

    if (!whatdepends->get_value().empty()) {
        auto matched_reldeps = libdnf5::rpm::ReldepList(ctx.base);
        for (const auto & reldep_glob : whatdepends->get_value()) {
            matched_reldeps.add_reldep_with_glob(reldep_glob);
        }

        // Filter requires by reldeps
        auto dependsquery = full_package_query;
        dependsquery.filter_requires(matched_reldeps, libdnf5::sack::QueryCmp::EQ);

        // Filter weak deps via reldeps
        auto recommends_reldep_query = full_package_query;
        recommends_reldep_query.filter_recommends(matched_reldeps, libdnf5::sack::QueryCmp::EQ);
        dependsquery |= recommends_reldep_query;
        auto enhances_reldep_query = full_package_query;
        enhances_reldep_query.filter_enhances(matched_reldeps, libdnf5::sack::QueryCmp::EQ);
        dependsquery |= enhances_reldep_query;
        auto supplements_reldep_query = full_package_query;
        supplements_reldep_query.filter_supplements(matched_reldeps, libdnf5::sack::QueryCmp::EQ);
        dependsquery |= supplements_reldep_query;
        auto suggests_reldep_query = full_package_query;
        suggests_reldep_query.filter_suggests(matched_reldeps, libdnf5::sack::QueryCmp::EQ);
        dependsquery |= suggests_reldep_query;

        if (!exactdeps->get_value()) {
            auto pkgs_from_resolved_nevras =
                resolve_nevras_to_packges(ctx.base, whatdepends->get_value(), full_package_query);

            // Filter requires by packages from resolved nevras
            auto what_requires_resolved_nevras = full_package_query;
            what_requires_resolved_nevras.filter_requires(pkgs_from_resolved_nevras);
            dependsquery |= what_requires_resolved_nevras;

            // Filter weak deps by packages from resolved nevras
            auto recommends_pkg_query = full_package_query;
            recommends_pkg_query.filter_recommends(pkgs_from_resolved_nevras, libdnf5::sack::QueryCmp::EQ);
            dependsquery |= recommends_pkg_query;
            auto enhances_pkg_query = full_package_query;
            enhances_pkg_query.filter_enhances(pkgs_from_resolved_nevras, libdnf5::sack::QueryCmp::EQ);
            dependsquery |= enhances_pkg_query;
            auto supplements_pkg_query = full_package_query;
            supplements_pkg_query.filter_supplements(pkgs_from_resolved_nevras, libdnf5::sack::QueryCmp::EQ);
            dependsquery |= supplements_pkg_query;
            auto suggests_pkg_query = full_package_query;
            suggests_pkg_query.filter_suggests(pkgs_from_resolved_nevras, libdnf5::sack::QueryCmp::EQ);
            dependsquery |= suggests_pkg_query;
        }
        //TODO(amatej): add recurisve option call

        full_package_query = dependsquery;
    }
    if (!whatprovides->get_value().empty()) {
        auto provides_query = full_package_query;
        provides_query.filter_provides(whatprovides->get_value(), libdnf5::sack::QueryCmp::GLOB);
        if (!provides_query.empty()) {
            full_package_query = provides_query;
        } else {
            // If provides query doesn't match anything try matching files
            full_package_query.filter_file(whatprovides->get_value(), libdnf5::sack::QueryCmp::GLOB);
        }
    }
    if (!whatrequires->get_value().empty()) {
        if (exactdeps->get_value()) {
            full_package_query.filter_requires(whatrequires->get_value(), libdnf5::sack::QueryCmp::GLOB);
        } else {
            auto requires_resolved = full_package_query;
            requires_resolved.filter_requires(
                resolve_nevras_to_packges(ctx.base, whatrequires->get_value(), full_package_query));

            full_package_query.filter_requires(whatrequires->get_value(), libdnf5::sack::QueryCmp::GLOB);
            full_package_query |= requires_resolved;
            //TODO(amatej): add recurisve option call
        }
    }
    if (!whatobsoletes->get_value().empty()) {
        full_package_query.filter_obsoletes(whatobsoletes->get_value(), libdnf5::sack::QueryCmp::GLOB);
    }
    if (!whatconflicts->get_value().empty()) {
        auto conflicts_resolved = full_package_query;
        conflicts_resolved.filter_conflicts(
            resolve_nevras_to_packges(ctx.base, whatconflicts->get_value(), full_package_query));

        full_package_query.filter_conflicts(whatconflicts->get_value(), libdnf5::sack::QueryCmp::GLOB);
        full_package_query |= conflicts_resolved;
    }
    if (!whatrecommends->get_value().empty()) {
        auto recommends_resolved = full_package_query;
        recommends_resolved.filter_recommends(
            resolve_nevras_to_packges(ctx.base, whatrecommends->get_value(), full_package_query));

        full_package_query.filter_recommends(whatrecommends->get_value(), libdnf5::sack::QueryCmp::GLOB);
        full_package_query |= recommends_resolved;
    }
    if (!whatenhances->get_value().empty()) {
        auto enhances_resolved = full_package_query;
        enhances_resolved.filter_enhances(
            resolve_nevras_to_packges(ctx.base, whatenhances->get_value(), full_package_query));

        full_package_query.filter_enhances(whatenhances->get_value(), libdnf5::sack::QueryCmp::GLOB);
        full_package_query |= enhances_resolved;
    }
    if (!whatsupplements->get_value().empty()) {
        auto supplements_resolved = full_package_query;
        supplements_resolved.filter_supplements(
            resolve_nevras_to_packges(ctx.base, whatsupplements->get_value(), full_package_query));

        full_package_query.filter_supplements(whatsupplements->get_value(), libdnf5::sack::QueryCmp::GLOB);
        full_package_query |= supplements_resolved;
    }
    if (!whatsuggests->get_value().empty()) {
        auto suggests_resolved = full_package_query;
        suggests_resolved.filter_suggests(
            resolve_nevras_to_packges(ctx.base, whatsuggests->get_value(), full_package_query));

        full_package_query.filter_suggests(whatsuggests->get_value(), libdnf5::sack::QueryCmp::GLOB);
        full_package_query |= suggests_resolved;
    }

    if (!arch->get_value().empty()) {
        full_package_query.filter_arch(arch->get_value(), libdnf5::sack::QueryCmp::GLOB);
    }

    if (!file->get_value().empty()) {
        full_package_query.filter_file(file->get_value(), libdnf5::sack::QueryCmp::GLOB);
    }

    if (duplicates->get_value()) {
        full_package_query -= get_installonly_query(ctx.base);
        full_package_query.filter_duplicates();
    }

    if (unneeded->get_value()) {
        full_package_query.filter_unneeded();
    }

    if (extras->get_value()) {
        full_package_query.filter_extras();
    }

    if (upgrades->get_value()) {
        full_package_query.filter_upgrades();
    }

    if (recent->get_value()) {
        auto & cfg_main = ctx.base.get_config();
        auto recent_limit_days = cfg_main.get_recent_option().get_value();
        auto now = time(nullptr);
        full_package_query.filter_recent(now - (recent_limit_days * 86400));
    }

    if (installonly->get_value()) {
        full_package_query &= get_installonly_query(ctx.base);
    }

    if (querytags_option->get_value()) {
        libdnf5::cli::output::print_available_pkg_attrs(stdout);
    } else if (changelogs->get_value()) {
        libdnf5::cli::output::print_changelogs(full_package_query, full_package_query, false, 0, 0);
    } else if (info_option->get_value()) {
        for (auto package : full_package_query) {
            libdnf5::cli::output::print_package_info_table(package);
            std::cout << '\n';
        }
    } else if (!pkg_attr_option->get_value().empty()) {
        libdnf5::cli::output::print_pkg_attr_uniq_sorted(stdout, full_package_query, pkg_attr_option->get_value());
    } else {
        libdnf5::cli::output::print_pkg_set_with_format(stdout, full_package_query, query_format_option->get_value());
    }
}

}  // namespace dnf5
