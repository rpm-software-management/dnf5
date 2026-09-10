// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "removevendorpolicy.hpp"

#include "shared.hpp"

#include <fmt/format.h>
#include <libdnf5/base/vendor_change_manager.hpp>
#include <libdnf5/common/sack/match_string.hpp>
#include <libdnf5/utils/format.hpp>

namespace dnf5 {

using namespace ::libdnf5;


void ConfigManagerRemoveVendorPolicyCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cmd.set_description(_("Remove vendor change policy files"));

    auto name_opt = parser.add_new_named_arg("name");
    name_opt->set_long_name("name");
    name_opt->set_has_value(true);
    name_opt->set_arg_value_help("PATTERN");
    name_opt->set_description(_("Policy base filename pattern (supports globs)"));
    name_opt->set_parse_hook_func([this](cli::ArgumentParser::NamedArg *, const char *, const char * value) {
        name_pattern = value;
        return true;
    });
    cmd.register_named_arg(name_opt);
}


void ConfigManagerRemoveVendorPolicyCommand::configure() {
    auto & ctx = get_context();
    auto & base = ctx.get_base();
    auto logger = base.get_logger();

    auto vcm = base.get_vendor_change_manager();

    // Get all policy files from VendorChangeManager
    auto visible_policy_files = vcm->get_policy_files();

    if (visible_policy_files.empty()) {
        throw ConfigManagerError(M_("No vendor policy files found"));
    }

    // Find files matching the patterns
    std::vector<std::filesystem::path> matched_files;
    for (const auto & file : visible_policy_files) {
        std::string base_name = vcm->extract_policy_base_filename(file);
        if (sack::match_string(base_name, sack::QueryCmp::GLOB, name_pattern)) {
            matched_files.push_back(file);
        }
    }

    if (matched_files.empty()) {
        throw ConfigManagerError(
            M_("No matching vendor policy files found for pattern: {pattern}"), NamedErrorArg("pattern", name_pattern));
    }

    // Remove the files using VendorChangeManager API
    for (const auto & file : matched_files) {
        const std::string base_name = vcm->extract_policy_base_filename(file);
        const auto masked_file = vcm->get_masked_policy_file(file);
        if (vcm->is_policy_file_manageable(file)) {
            try {
                auto path = vcm->remove_policy_file(base_name);
                ctx.print_info(
                    utils::sformat(_("Removed vendor policy file: {path}"), fmt::arg("path", path.string())));
                logger->info("config-manager: Removed vendor policy file: {}", path.string());
                if (!masked_file.empty()) {
                    ctx.print_info(utils::sformat(
                        _("Unmasked distribution policy file: {path}"), fmt::arg("path", masked_file.string())));
                    logger->info("config-manager: Unmasked distribution policy file: {}", masked_file.string());
                }
            } catch (const libdnf5::Error & e) {
                libdnf5::throw_with_nested(ConfigManagerError(
                    M_("Failed to remove policy file: {path}"), NamedErrorArg("path", file.string())));
            }
        } else {
            ctx.print_info(utils::sformat(
                _("Cannot remove distribution policy file \"{path}\". "
                  "To mask it, use: {command}"),
                fmt::arg("path", file.string()),
                fmt::arg(
                    "command", "dnf5 config-manager add-vendor-policy --name='" + base_name + "' --mask --policy=''")));
        }
    }
}

}  // namespace dnf5
