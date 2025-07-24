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

#include "do.hpp"

#include <dnf5/shared_options.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

namespace dnf5 {

using namespace libdnf5::cli;

void DoCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}

void DoCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Do transaction"));

    {
        auto action_opt = parser.add_new_named_arg("action");
        action_opt->set_long_name("action");
        action_opt->set_description(_("Action to be done on the following items."));
        action_opt->set_has_value(true);
        action_opt->set_arg_value_help("install/remove/upgrade/downgrade/reinstall");
        action_opt->set_parse_hook_func(
            [this](ArgumentParser::NamedArg *, [[maybe_unused]] const char * option, const char * value) {
                static const std::map<std::string_view, Action> ACTION_TO_ENUM_MAP{
                    {"install", Action::INSTALL},
                    {"remove", Action::REMOVE},
                    {"upgrade", Action::UPGRADE},
                    {"downgrade", Action::DOWNGRADE},
                    {"reinstall", Action::REINSTALL}};
                if (auto it = ACTION_TO_ENUM_MAP.find(value); it != ACTION_TO_ENUM_MAP.end()) {
                    action = it->second;
                } else {
                    throw ArgumentParserInvalidValueError(M_("Unsupported action \"{}\""), std::string(value));
                }
                return true;
            });
        cmd.register_named_arg(action_opt);
    }

    {
        auto item_type_opt = parser.add_new_named_arg("type");
        item_type_opt->set_long_name("type");
        item_type_opt->set_description(_("Type of the following items."));
        item_type_opt->set_has_value(true);
        item_type_opt->set_arg_value_help("auto/package/group");
        item_type_opt->set_parse_hook_func(
            [this](ArgumentParser::NamedArg *, [[maybe_unused]] const char * option, const char * value) {
                static const std::map<std::string_view, ItemType> TYPE_TO_ENUM_MAP{
                    {"auto", ItemType::AUTO},
                    {"package", ItemType::PACKAGE},
                    {"group", ItemType::GROUP},
                    {"environment", ItemType::ENVIRONMENT}};
                if (auto it = TYPE_TO_ENUM_MAP.find(value); it != TYPE_TO_ENUM_MAP.end()) {
                    item_type = it->second;
                } else {
                    throw ArgumentParserInvalidValueError(M_("Unsupported item type \"{}\""), std::string(value));
                }
                return true;
            });
        cmd.register_named_arg(item_type_opt);
    }

    create_installed_from_repo_option(*this, installed_from_repos, false);
    create_from_repo_option(*this, from_repos, false);

    {
        auto items =
            parser.add_new_positional_arg("items", ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
        items->set_description(_("List of items to take action on"));
        items->set_parse_hook_func([this](
                                       [[maybe_unused]] ArgumentParser::PositionalArg * arg,
                                       int argc,
                                       const char * const argv[]) {
            libdnf5::GoalJobSettings settings;
            settings.set_from_repo_ids(installed_from_repos);
            settings.set_to_repo_ids(from_repos);
            switch (action) {
                case Action::INSTALL:
                    for (int i = 0; i < argc; ++i) {
                        install_items.emplace_back(item_type, argv[i], settings);
                    }
                    break;
                case Action::REMOVE:
                    // Limit remove spec capabity to prevent multiple matches. Remove command should not match anything after performing
                    // a remove action with the same spec. NEVRA and filenames are the only types that have no overlaps.
                    settings.set_with_nevra(true);
                    settings.set_with_provides(false);
                    settings.set_with_filenames(true);
                    settings.set_with_binaries(false);
                    for (int i = 0; i < argc; ++i) {
                        remove_items.emplace_back(item_type, argv[i], settings);
                    }
                    break;
                case Action::UPGRADE:
                    for (int i = 0; i < argc; ++i) {
                        upgrade_items.emplace_back(item_type, argv[i], settings);
                    }
                    break;
                case Action::DOWNGRADE:
                    if (item_type != ItemType::AUTO && item_type != ItemType::PACKAGE) {
                        throw ArgumentParserInvalidValueError(M_("Only package downgrade is supported."));
                    }
                    for (int i = 0; i < argc; ++i) {
                        if (argv[i][0] == '@') {
                            throw ArgumentParserInvalidValueError(M_("Only package downgrade is supported."));
                        }
                        downgrade_items.emplace_back(item_type, argv[i], settings);
                    }
                    break;
                case Action::REINSTALL:
                    if (item_type != ItemType::AUTO && item_type != ItemType::PACKAGE) {
                        throw ArgumentParserInvalidValueError(M_("Only package reinstall is supported."));
                    }
                    for (int i = 0; i < argc; ++i) {
                        if (argv[i][0] == '@') {
                            throw ArgumentParserInvalidValueError(M_("Only package reinstall is supported."));
                        }
                        reinstall_items.emplace_back(item_type, argv[i], settings);
                    }
                    break;
                case Action::UNDEFINED:
                    throw ArgumentParserInvalidValueError(M_("Action was not defined"));
            }
            return true;
        });
        items->set_nrepeats(ArgumentParser::PositionalArg::UNLIMITED);
        items->set_complete_hook_func(
            [&ctx](const char * arg) { return match_specs(ctx, arg, false, true, true, false); });
        cmd.register_positional_arg(items);
    }

    allow_erasing = std::make_unique<AllowErasingOption>(*this);
    auto skip_broken = std::make_unique<SkipBrokenOption>(*this);
    auto skip_unavailable = std::make_unique<SkipUnavailableOption>(*this);
    create_allow_downgrade_options(*this);

    advisory_name = std::make_unique<AdvisoryOption>(*this);
    advisory_severity = std::make_unique<AdvisorySeverityOption>(*this);
    advisory_bz = std::make_unique<BzOption>(*this);
    advisory_cve = std::make_unique<CveOption>(*this);
    advisory_security = std::make_unique<SecurityOption>(*this);
    advisory_bugfix = std::make_unique<BugfixOption>(*this);
    advisory_enhancement = std::make_unique<EnhancementOption>(*this);
    advisory_newpackage = std::make_unique<NewpackageOption>(*this);

    create_downloadonly_option(*this);
    create_offline_option(*this);
    create_store_option(*this);
}

void DoCommand::configure() {
    auto & context = get_context();

    const auto in_pkgs_count =
        install_items.size() + upgrade_items.size() + downgrade_items.size() + reinstall_items.size();

    std::vector<std::string> pkg_specs;
    pkg_specs.reserve(in_pkgs_count);
    for (const auto & items : {install_items, upgrade_items, downgrade_items, reinstall_items}) {
        for (const auto & item : items) {
            pkg_specs.emplace_back(item.spec);
        }
    }
    context.update_repo_metadata_from_specs(pkg_specs);

    context.set_load_system_repo(true);

    context.update_repo_metadata_from_advisory_options(
        advisory_name->get_value(),
        advisory_security->get_value(),
        advisory_bugfix->get_value(),
        advisory_enhancement->get_value(),
        advisory_newpackage->get_value(),
        advisory_severity->get_value(),
        advisory_bz->get_value(),
        advisory_cve->get_value());

    context.set_load_available_repos(
        in_pkgs_count > 0 ? Context::LoadAvailableRepos::ENABLED : Context::LoadAvailableRepos::NONE);
}


std::string DoCommand::group_spec(ItemType type, const std::string & spec, libdnf5::GoalJobSettings & settings) {
    if (type == ItemType::GROUP) {
        settings.set_group_search_environments(false);
        settings.set_group_search_groups(true);
    } else {
        settings.set_group_search_environments(true);
        settings.set_group_search_groups(false);
    }
    if (spec.starts_with("id=")) {
        settings.set_group_with_id(true);
        settings.set_group_with_name(false);
        return spec.substr(3);
    } else if (spec.starts_with("name=")) {
        settings.set_group_with_id(false);
        settings.set_group_with_name(true);
        return spec.substr(5);
    }
    throw std::runtime_error(libdnf5::utils::sformat(_("Invalid Environmet/Group specification: {}"), spec));
}


void DoCommand::run() {
    auto & ctx = get_context();
    auto goal = get_context().get_goal();
    goal->set_allow_erasing(allow_erasing->get_value());
    auto advisories = advisory_query_from_cli_input(
        ctx.get_base(),
        advisory_name->get_value(),
        advisory_security->get_value(),
        advisory_bugfix->get_value(),
        advisory_enhancement->get_value(),
        advisory_newpackage->get_value(),
        advisory_severity->get_value(),
        advisory_bz->get_value(),
        advisory_cve->get_value(),
        !ctx.get_base().get_config().get_skip_unavailable_option().get_value());

    for (auto & item : install_items) {
        if (advisories) {
            item.settings.set_advisory_filter(advisories.value());
        }
        switch (item.type) {
            case ItemType::AUTO:
                goal->add_install(item.spec, item.settings);
                break;
            case ItemType::PACKAGE:
                goal->add_rpm_install(item.spec, item.settings);
                break;
            case ItemType::GROUP:
            case ItemType::ENVIRONMENT:
                auto spec = group_spec(item.type, item.spec, item.settings);
                goal->add_group_install(spec, libdnf5::transaction::TransactionItemReason::USER, item.settings);
                break;
        }
    }

    for (auto & item : remove_items) {
        switch (item.type) {
            case ItemType::AUTO:
                goal->add_remove(item.spec, item.settings);
                break;
            case ItemType::PACKAGE:
                goal->add_rpm_remove(item.spec, item.settings);
                break;
            case ItemType::GROUP:
            case ItemType::ENVIRONMENT:
                auto spec = group_spec(item.type, item.spec, item.settings);
                goal->add_group_remove(spec, libdnf5::transaction::TransactionItemReason::USER, item.settings);
                break;
        }
    }

    for (auto & item : upgrade_items) {
        if (advisories) {
            item.settings.set_advisory_filter(advisories.value());
        }
        switch (item.type) {
            case ItemType::AUTO:
                goal->add_upgrade(item.spec, item.settings);
                break;
            case ItemType::PACKAGE:
                goal->add_rpm_upgrade(item.spec, item.settings);
                break;
            case ItemType::GROUP:
            case ItemType::ENVIRONMENT:
                auto spec = group_spec(item.type, item.spec, item.settings);
                goal->add_group_upgrade(spec, item.settings);
                break;
        }
    }

    for (auto & item : downgrade_items) {
        switch (item.type) {
            case ItemType::AUTO:
                goal->add_downgrade(item.spec, item.settings);
                break;
            case ItemType::PACKAGE:
                goal->add_rpm_downgrade(item.spec, item.settings);
                break;
            case ItemType::GROUP:
            case ItemType::ENVIRONMENT:
                throw std::runtime_error(_("Downgrading environments and groups is not supported."));
                break;
        }
    }

    for (const auto & item : reinstall_items) {
        goal->add_reinstall(item.spec, item.settings);
        switch (item.type) {
            case ItemType::AUTO:
                goal->add_reinstall(item.spec, item.settings);
                break;
            case ItemType::PACKAGE:
                goal->add_rpm_reinstall(item.spec, item.settings);
                break;
            case ItemType::GROUP:
            case ItemType::ENVIRONMENT:
                throw std::runtime_error(_("Reinstaling environments and groups is not supported."));
                break;
        }
    }
}

}  // namespace dnf5
