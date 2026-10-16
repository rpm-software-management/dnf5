// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "addvendorpolicy.hpp"

#include "shared.hpp"

#include <curl/curl.h>
#include <fmt/format.h>
#include <libdnf5/base/vendor_change_manager.hpp>
#include <libdnf5/repo/file_downloader.hpp>
#include <libdnf5/utils/fs/temp.hpp>

namespace dnf5 {

using namespace ::libdnf5;

namespace {

// Extracts a specific part of the URL from a URL string.
// Returns an empty string if the URL is not valid/supported or the required part is not present.
std::string get_url_part(const std::string & url, CURLUPart what_part) {
    std::string ret;
    CURLUcode rc;
    CURLU * c_url = curl_url();
    rc = curl_url_set(c_url, CURLUPART_URL, url.c_str(), 0);
    if (!rc) {
        char * part;
        rc = curl_url_get(c_url, what_part, &part, 0);
        if (!rc) {
            ret = part;
            curl_free(part);
        }
    }
    curl_url_cleanup(c_url);
    return ret;
}

}  // namespace


void ConfigManagerAddVendorPolicyCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cmd.set_description(_("Add a vendor change policy from a TOML file or compact format string"));

    auto name_opt = parser.add_new_named_arg("name");
    name_opt->set_long_name("name");
    name_opt->set_has_value(true);
    name_opt->set_arg_value_help("POLICY_NAME");
    name_opt->set_description(_("Policy base filename (without .conf extension)"));
    name_opt->set_parse_hook_func([this](cli::ArgumentParser::NamedArg *, const char *, const char * value) {
        policy_name = value;
        return true;
    });
    cmd.register_named_arg(name_opt);

    auto from_tomlfile_opt = parser.add_new_named_arg("from-tomlfile");
    from_tomlfile_opt->set_long_name("from-tomlfile");
    from_tomlfile_opt->set_has_value(true);
    from_tomlfile_opt->set_arg_value_help("URL_OR_PATH");
    from_tomlfile_opt->set_description(_("URL or local path to the source TOML policy file"));
    from_tomlfile_opt->set_parse_hook_func([this](cli::ArgumentParser::NamedArg *, const char *, const char * value) {
        source_location = value;
        is_local_path = get_url_part(source_location, CURLUPART_SCHEME) == "";
        return true;
    });
    cmd.register_named_arg(from_tomlfile_opt);

    auto policy_opt = parser.add_new_named_arg("policy");
    policy_opt->set_long_name("policy");
    policy_opt->set_has_value(true);
    policy_opt->set_arg_value_help("COMPACT_POLICY_STRING");
    policy_opt->set_description(_("Vendor change policy in compact format"));
    policy_opt->set_parse_hook_func([this](cli::ArgumentParser::NamedArg *, const char *, const char * value) {
        compact_policy = value;
        return true;
    });
    cmd.register_named_arg(policy_opt);

    // Make --from-tomlfile and --policy mutually exclusive
    from_tomlfile_opt->add_conflict_argument(*policy_opt);

    auto allow_mask_opt = parser.add_new_named_arg("allow-mask");
    allow_mask_opt->set_long_name("allow-mask");
    allow_mask_opt->set_description(_("Allow masking a distribution policy"));
    allow_mask_opt->set_parse_hook_func([this](cli::ArgumentParser::NamedArg *, const char *, const char *) {
        allow_mask = true;
        return true;
    });
    cmd.register_named_arg(allow_mask_opt);

    auto allow_replace_opt = parser.add_new_named_arg("allow-replace");
    allow_replace_opt->set_long_name("allow-replace");
    allow_replace_opt->set_description(_("Allow replacing an existing policy file"));
    allow_replace_opt->set_parse_hook_func([this](cli::ArgumentParser::NamedArg *, const char *, const char *) {
        allow_replace = true;
        return true;
    });
    cmd.register_named_arg(allow_replace_opt);

    auto mask_opt = parser.add_new_named_arg("mask");
    mask_opt->set_long_name("mask");
    mask_opt->set_description(_("Require that the new policy masks a distribution policy"));
    mask_opt->set_parse_hook_func([this](cli::ArgumentParser::NamedArg *, const char *, const char *) {
        mask = true;
        return true;
    });
    cmd.register_named_arg(mask_opt);

    // Make --allow-mask and --mask mutually exclusive
    allow_mask_opt->add_conflict_argument(*mask_opt);

    auto replace_opt = parser.add_new_named_arg("replace");
    replace_opt->set_long_name("replace");
    replace_opt->set_description(_("Require that the new policy replaces an existing system policy file"));
    replace_opt->set_parse_hook_func([this](cli::ArgumentParser::NamedArg *, const char *, const char *) {
        replace = true;
        return true;
    });
    cmd.register_named_arg(replace_opt);

    // Make --allow-replace and --replace mutually exclusive
    allow_replace_opt->add_conflict_argument(*replace_opt);
}


void ConfigManagerAddVendorPolicyCommand::configure() {
    auto & ctx = get_context();
    auto & base = ctx.get_base();
    auto logger = base.get_logger();

    if (policy_name.empty()) {
        throw ConfigManagerError(
            M_("Option \"{option}\" with nonempty value is required"), NamedErrorArg("option", "--name"));
    }

    // Exactly one of --policy or --from-tomlfile must be specified
    if (!compact_policy && source_location.empty()) {
        throw ConfigManagerError(
            M_("Either \"{option1}\" or non-empty \"{option2}\" is required"),
            NamedErrorArg("option1", "--policy"),
            NamedErrorArg("option2", "--from-tomlfile"));
    }

    auto vcm = base.get_vendor_change_manager();

    const auto path = vcm->find_policy_file(policy_name);
    if (!path.empty()) {
        if (vcm->is_policy_file_manageable(path)) {
            // A system policy file exists
            if (!allow_replace && !replace) {
                throw ConfigManagerError(
                    M_("Policy file \"{path}\" already exists. Use \"{option}\" to allow replacing it."),
                    NamedErrorArg("path", path.string()),
                    NamedErrorArg("option", "--allow-replace"));
            }
        } else {
            // A distribution policy file exists
            if (!allow_mask && !mask) {
                throw ConfigManagerError(
                    M_("Distribution policy \"{name}\" already exists. Use \"{option}\" to allow masking it."),
                    NamedErrorArg("name", policy_name),
                    NamedErrorArg("option", "--allow-mask"));
            }
        }
    }

    const auto distribution_policy_file =
        path.empty() ? "" : (!vcm->is_policy_file_manageable(path) ? path : vcm->get_masked_policy_file(path));

    const bool distribution_policy_exists = !distribution_policy_file.empty();
    const bool managed_policy_exists = !path.empty() && vcm->is_policy_file_manageable(path);

    if (mask && !distribution_policy_exists) {
        throw ConfigManagerError(
            M_("Cannot add policy: option \"{option}\" requires masking a distribution policy, "
               "but no distribution policy named \"{name}\" exists."),
            NamedErrorArg("option", "--mask"),
            NamedErrorArg("name", policy_name));
    }
    if (replace && !managed_policy_exists) {
        throw ConfigManagerError(
            M_("Cannot add policy: option \"{option}\" requires replacing an existing system policy, "
               "but no system policy file named \"{name}\" exists."),
            NamedErrorArg("option", "--replace"),
            NamedErrorArg("name", policy_name));
    }

    try {
        if (compact_policy) {
            // Save policy from compact format
            // VendorChangeManager will convert to TOML and save
            auto dest_path = vcm->save_policy_from_compact(
                *compact_policy, "text:COMMAND LINE", policy_name, allow_replace || replace);

            if (dest_path == path) {
                ctx.print_info(
                    utils::sformat(_("Replaced vendor policy file: {path}"), fmt::arg("path", dest_path.string())));
                logger->info(
                    "config-manager: Replaced vendor policy file \"{}\" from compact string", dest_path.string());
            } else {
                ctx.print_info(
                    utils::sformat(_("Created vendor policy file: {path}"), fmt::arg("path", dest_path.string())));
                logger->info(
                    "config-manager: Created vendor policy file \"{}\" from compact string", dest_path.string());
            }
        } else if (is_local_path) {
            // Save policy from local TOML file
            if (!std::filesystem::exists(source_location)) {
                throw ConfigManagerError(
                    M_("Source file does not exist: {path}"), NamedErrorArg("path", source_location));
            }

            auto dest_path = vcm->save_policy_from_toml(source_location, policy_name, allow_replace || replace);

            if (dest_path == path) {
                ctx.print_info(
                    utils::sformat(_("Replaced vendor policy file: {path}"), fmt::arg("path", dest_path.string())));
                logger->info(
                    "config-manager: Replaced vendor policy file \"{}\" from file \"{}\"",
                    dest_path.string(),
                    source_location);
            } else {
                ctx.print_info(
                    utils::sformat(_("Saved vendor policy file: {path}"), fmt::arg("path", dest_path.string())));
                logger->info(
                    "config-manager: Saved vendor policy file \"{}\" from file \"{}\"",
                    dest_path.string(),
                    source_location);
            }
        } else {
            // Download and save policy from URL defined file
            utils::fs::TempFile temp_file("policy.conf");

            repo::FileDownloader downloader(base);
            downloader.add(source_location, temp_file.get_path());
            downloader.download();

            auto dest_path = vcm->save_policy_from_toml(temp_file.get_path(), policy_name, allow_replace || replace);

            if (dest_path == path) {
                ctx.print_info(
                    utils::sformat(_("Replaced vendor policy file: {path}"), fmt::arg("path", dest_path.string())));
                logger->info(
                    "config-manager: Replaced vendor policy file \"{}\" from url \"{}\"",
                    dest_path.string(),
                    source_location);
            } else {
                ctx.print_info(
                    utils::sformat(_("Saved vendor policy file: {path}"), fmt::arg("path", dest_path.string())));
                logger->info(
                    "config-manager: Saved vendor policy file \"{}\" from url \"{}\"",
                    dest_path.string(),
                    source_location);
            }
        }
        if (distribution_policy_exists) {
            ctx.print_info(utils::sformat(
                _("Masked distribution policy file: {path}"), fmt::arg("path", distribution_policy_file.string())));
            logger->info("config-manager: Masked distribution policy file: {}", distribution_policy_file.string());
        }
    } catch (const libdnf5::Error & e) {
        libdnf5::throw_with_nested(ConfigManagerError(M_("Failed to add vendor policy")));
    }
}

}  // namespace dnf5
