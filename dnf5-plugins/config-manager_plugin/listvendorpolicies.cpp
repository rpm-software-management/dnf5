// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "listvendorpolicies.hpp"

#include "shared.hpp"

#include <libdnf5-cli/tty.hpp>
#include <libdnf5/base/vendor_change_manager.hpp>
#include <libdnf5/common/sack/match_string.hpp>
#include <libdnf5/utils/format.hpp>
#include <libsmartcols/libsmartcols.h>

#include <iostream>
#include <ranges>
#include <string_view>

namespace dnf5 {

using namespace ::libdnf5;


void ConfigManagerListVendorPoliciesCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cmd.set_description(_("List vendor change policy files"));

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

    auto info_opt = parser.add_new_named_arg("info");
    info_opt->set_long_name("info");
    info_opt->set_description(_("Show detailed information about the policies"));
    info_opt->set_parse_hook_func([this](cli::ArgumentParser::NamedArg *, const char *, const char *) {
        info = true;
        return true;
    });
    cmd.register_named_arg(info_opt);
}


void ConfigManagerListVendorPoliciesCommand::configure() {
    auto & ctx = get_context();
    auto & base = ctx.get_base();

    auto vcm = base.get_vendor_change_manager();

    // Get all policy files from VendorChangeManager
    auto visible_policy_files = vcm->get_policy_files();

    if (visible_policy_files.empty()) {
        throw ConfigManagerError(M_("No vendor policy files found"));
    }

    // If pattern is specified, filter the files
    std::vector<std::filesystem::path> matched_files;
    if (!name_pattern.empty()) {
        for (const auto & file : visible_policy_files) {
            std::string base_name = vcm->extract_policy_base_filename(file);
            if (sack::match_string(base_name, sack::QueryCmp::GLOB, name_pattern)) {
                matched_files.push_back(file);
            }
        }
    } else {
        // No pattern - use all files
        matched_files = std::move(visible_policy_files);
    }

    if (matched_files.empty()) {
        throw ConfigManagerError(
            M_("No matching vendor policy files found for pattern: {pattern}"), NamedErrorArg("pattern", name_pattern));
    }

    if (!info) {
        // Simple listing
        for (const auto & file : matched_files) {
            const auto is_manageable = vcm->is_policy_file_manageable(file);
            if (is_manageable) {
                std::cout << "+ " << file.string() << std::endl;
                const auto masked_file = vcm->get_masked_policy_file(file);
                if (!masked_file.empty()) {
                    std::cout << "- " << masked_file.string() << std::endl;
                }
            } else {
                std::cout << "* " << file.string() << std::endl;
            }
        }
    } else {
        // Detailed listing with --info
        bool first = true;
        for (const auto & file : matched_files) {
            if (!first) {
                std::cout << std::endl;
            }
            first = false;

            const auto masked_file = vcm->get_masked_policy_file(file);
            print_policy_info(file, masked_file);
            if (!masked_file.empty()) {
                std::cout << std::endl;
                print_policy_info(masked_file, file);
            }
        }
    }
}


namespace {

enum { COL_KEY, COL_VALUE };

struct libscols_line * add_line(struct libscols_table * table, const std::string & key, const std::string & value) {
    struct libscols_line * ln = scols_table_new_line(table, NULL);
    scols_line_set_data(ln, COL_KEY, key.c_str());
    scols_line_set_data(ln, COL_VALUE, value.c_str());
    return ln;
}

}  // namespace


void ConfigManagerListVendorPoliciesCommand::print_policy_info(
    const std::filesystem::path & policy_file, const std::filesystem::path & mask_file) {
    auto & ctx = get_context();
    auto & base = ctx.get_base();

    auto vcm = base.get_vendor_change_manager();

    // Detailed listing with --info
    const auto base_filename = vcm->extract_policy_base_filename(policy_file);
    const auto is_manageable = vcm->is_policy_file_manageable(policy_file);

    // Setup table
    struct libscols_table * table = scols_new_table();
    scols_table_enable_noheadings(table, 1);
    scols_table_new_column(table, "key", 1, 0);
    scols_table_new_column(table, "value", 1, SCOLS_FL_WRAP);
    // Note for translators: This is a right-aligned column separator in
    // a package properties table as in "Name    : bash".
    scols_table_set_column_separator(table, _(" : "));
    if (libdnf5::cli::tty::is_coloring_enabled()) {
        scols_table_enable_colors(table, 1);
    }

    // Add policy info
    add_line(table, _("Policy"), base_filename.string());
    add_line(table, _("Source"), policy_file.string());
    add_line(table, _("Type"), is_manageable ? _("System (manageable)") : _("Distribution (read-only)"));
    if (is_manageable) {
        if (!mask_file.empty()) {
            add_line(table, _("Masks"), mask_file.string());
        }
    } else {
        // Check if this distribution policy is masked by a system policy
        if (!mask_file.empty()) {
            add_line(table, _("Masked by"), mask_file.string());
        }
    }

    // Get compact representation
    std::string policy_compact;
    try {
        policy_compact = vcm->convert_policy_toml_to_compact(policy_file);
    } catch (const libdnf5::Error & e) {
        policy_compact = utils::sformat(_("<conversion failed: {what}>"), fmt::arg("what", e.what()));
    }

    if (policy_compact.empty()) {
        add_line(table, _("Compact"), "");
    } else {
        bool first = true;
        for (auto line_range : std::views::split(policy_compact, '\n')) {
            std::string line(line_range.begin(), line_range.end());
            add_line(table, first ? _("Compact") : "", line);
            first = false;
        }
    }

    scols_print_table(table);
    scols_unref_table(table);
}

}  // namespace dnf5
