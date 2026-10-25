// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "copr.hpp"
#include "copr_config.hpp"
#include "copr_repo.hpp"

#include <libdnf5/base/vendor_change_manager.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/conf/option_bool.hpp>
#include <libdnf5/conf/option_errors.hpp>

#include <iostream>
#include <optional>

using namespace libdnf5::cli;

namespace dnf5 {

namespace {

// Builds a compact policy string with one "<direction>:[repoid=...]" entry per enabled
// repository, e.g. "@in:[repoid=\"A\"],in:[repoid=\"B\"]". Returns an empty string if `enabled`
// is false, so the resulting policy has no effect in that direction.
std::string build_directional_policy_str(
    bool enabled, const char * direction, const std::vector<std::string> & enabled_repoids) {
    if (!enabled) {
        return "";
    }
    std::string policy_str;
    for (const auto & repoid : enabled_repoids) {
        policy_str += policy_str.empty() ? '@' : ',';
        policy_str += libdnf5::utils::sformat(R"({}:[repoid="{}"])", direction, repoid);
    }
    return policy_str;
}


// Saves `policy_str` (as the `base_filename` vendor change policy) and reports the outcome.
// A failure to save is reported as a warning rather than propagated, so one direction's
// failure does not prevent the other from being attempted.
void save_vendor_policy(libdnf5::Base & base, const std::string & policy_str, const std::string & base_filename) {
    auto vcm = base.get_vendor_change_manager();
    std::filesystem::path path;
    bool policy_saved = false;
    try {
        // The vendor directory may not exist yet, e.g. on a minimal --installroot.
        vcm->ensure_vendor_conf_dir_exists();
        path = vcm->save_policy_from_compact(policy_str, "copr_plugin", base_filename, true);
        policy_saved = true;
    } catch (const libdnf5::Error & e) {
        warning(_("Failed to save vendor policy file '{}': {}"), base_filename, e.what());
    }
    if (policy_saved) {
        std::cerr << libdnf5::utils::sformat(_("Saved vendor policy file: {}\n"), path.string());
    }
}


// Human-readable description of a vendor change choice (1-4), matching the interactive menu options.
const char * vendor_change_choice_description(CoprConfig::VendorChangeChoice choice) {
    switch (choice) {
        case CoprConfig::VendorChangeChoice::TO_COPR_REPO:
            return _(
                "allow packages from these repositories to replace an already installed "
                "package from a different vendor");
        case CoprConfig::VendorChangeChoice::FROM_COPR_REPO:
            return _(
                "allow a package installed from these repositories to be replaced "
                "by a package from a different vendor");
        case CoprConfig::VendorChangeChoice::BOTH:
            return _("allow a vendor change in both directions");
        default:
            return _("do not allow a vendor change in either direction");
    }
}


// Determines whether a vendor change to/from packages of the repositories enabled by the 'enable'
// command should be allowed -- either from `forced_choice` (--vendor-change), or (if not given)
// from the Copr configuration's "vendor_change" setting (NONE/1 if unset), used directly with
// --assumeyes or as the interactive prompt's default otherwise -- and writes the corresponding
// vendor change policy/policies (empty ones if not allowed).
//
// Within a single vendor change policy, its outgoing_packages and incoming_packages filters are
// ANDed together (both must match for the policy to apply) -- so "allow both directions" cannot be
// expressed as one policy with both an "in:" and an "out:" filter; that would only permit replacing
// a package installed from these repositories with another package also from these repositories.
// Allowing both directions independently therefore requires two separate policy files, prefixed
// "i" and "o" (a prefix, rather than a suffix, cannot collide with a real base filename, since
// those always start with "_copr:").
void ask_and_apply_vendor_change_policy(
    libdnf5::Base & base,
    const std::vector<std::string> & enabled_repoids,
    const std::string & base_filename,
    std::optional<CoprConfig::VendorChangeChoice> forced_choice,
    CoprConfig::VendorChangeChoice default_choice) {
    auto & config = base.get_config();
    const bool assume_yes = config.get_assumeyes_option().get_value();

    using VendorChoice = CoprConfig::VendorChangeChoice;

    VendorChoice choice;
    std::cerr << std::endl;
    if (forced_choice) {
        choice = *forced_choice;
        std::cerr << libdnf5::utils::sformat(
            _("Using --vendor-change for the repositories enabled by this command: {}\n"),
            vendor_change_choice_description(choice));
    } else if (assume_yes) {
        choice = default_choice;
        std::cerr << libdnf5::utils::sformat(
            _("Using the vendor change setting from the Copr configuration (or its default) for the\n"
              "repositories enabled by this command: {}\n"),
            vendor_change_choice_description(choice));
    } else {
        choice = default_choice;
        std::cerr << _(
            "Packages from the repositories enabled by this command can be allowed to\n"
            "replace, or be replaced by, packages from a different vendor during package\n"
            "replacement (upgrade/downgrade/reinstall) operations:\n");
        std::cerr << _("  1. Do not allow a vendor change in either direction\n");
        std::cerr << _(
            "  2. Allow packages from the repositories enabled by this command to\n"
            "     replace an already installed package from a different vendor\n");
        std::cerr << _(
            "  3. Allow a package installed from the repositories enabled by this\n"
            "     command to be replaced by a package from a different vendor\n");
        std::cerr << _("  4. Allow both of the above (vendor change allowed in both directions)\n");

        while (true) {
            std::cerr << libdnf5::utils::sformat(_("Enter your choice [1-4, default {}]: "), static_cast<int>(choice));

            std::string line;
            if (!std::getline(std::cin, line) || line.empty()) {
                break;
            }

            try {
                std::size_t pos;
                const auto choice_num = std::stoi(line, &pos);
                if (pos == line.size() && choice_num >= 1 && choice_num <= 4) {
                    choice = static_cast<VendorChoice>(choice_num);
                    break;
                }
            } catch (const std::exception &) {
            }

            std::cerr << _("Invalid choice, please enter a number between 1 and 4.\n");
        }
    }

    const bool allow_incoming = (choice == VendorChoice::TO_COPR_REPO || choice == VendorChoice::BOTH);
    const bool allow_outgoing = (choice == VendorChoice::FROM_COPR_REPO || choice == VendorChoice::BOTH);

    // One "in:[repoid=...]" and/or "out:[repoid=...]" entry per enabled repository, so each
    // policy covers the main repository as well as any enabled dependency repositories.
    const auto incoming_policy_str = build_directional_policy_str(allow_incoming, "in", enabled_repoids);
    const auto outgoing_policy_str = build_directional_policy_str(allow_outgoing, "out", enabled_repoids);

    save_vendor_policy(base, incoming_policy_str, "i" + base_filename);
    save_vendor_policy(base, outgoing_policy_str, "o" + base_filename);
}

}  // namespace


void CoprEnableCommand::set_argument_parser() {
    CoprSubCommandWithID::set_argument_parser();
    auto & ctx = get_context();
    auto & cmd = *get_argument_parser_command();
    auto & parser = ctx.get_argument_parser();
    auto & base = ctx.get_base();

    std::string desc = libdnf5::utils::sformat(
        _("download the repository info from a Copr server and install it as a {}/*.repo file"),
        copr_repo_directory(&base).native());

    cmd.set_description(desc);
    cmd.set_long_description(desc);

    auto chroot = parser.add_new_positional_arg("CHROOT", ArgumentParser::PositionalArg::OPTIONAL, nullptr, nullptr);
    chroot->set_description(
        _("Chroot specified in the NAME-RELEASE-ARCH format, "
          "e.g. 'fedora-rawhide-ppc64le'.  When not specified, "
          "the 'dnf copr' command attempts to detect it."));
    chroot->set_parse_hook_func([this](
                                    [[maybe_unused]] ArgumentParser::PositionalArg * arg,
                                    [[maybe_unused]] int argc,
                                    const char * const argv[]) {
        opt_chroot = argv[0];
        return true;
    });
    cmd.register_positional_arg(chroot);

    auto vendor_change = parser.add_new_named_arg("vendor-change");
    vendor_change->set_long_name("vendor-change");
    vendor_change->set_has_value(true);
    vendor_change->set_arg_value_help("to-repo|from-repo|true|false");
    vendor_change->set_description(
        _("Allow a vendor change for packages of the repositories enabled by this command, "
          "without asking interactively. \"to-repo\" allows packages from these repositories "
          "to replace an already installed package from a different vendor; \"from-repo\" "
          "allows a package installed from these repositories to be replaced by a package "
          "from a different vendor; \"true\" allows both; \"false\" allows neither (same as "
          "the interactive default)."));
    vendor_change->set_parse_hook_func(
        [this](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            const std::string value_str{value};
            try {
                auto choice = CoprConfig::vendor_change_choice_from_string(value_str);
                opt_vendor_change_choice = choice;
            } catch (const libdnf5::OptionInvalidValueError &) {
                throw ArgumentParserError(
                    M_("--vendor-change: Invalid value \"{}\" (expected \"to-repo\", \"from-repo\", "
                       "or a boolean value)"),
                    value_str);
            }
            return true;
        });
    cmd.register_named_arg(vendor_change);
}


void CoprEnableCommand::run() {
    auto & base = get_context().get_base();
    std::unique_ptr<dnf5::CoprConfig> config = std::make_unique<dnf5::CoprConfig>(base);
    // Only consult the Copr configuration when --vendor-change was not given explicitly, so an
    // invalid "vendor_change" setting cannot block an --vendor-change override from taking effect.
    const auto vendor_change_default_choice =
        opt_vendor_change_choice ? *opt_vendor_change_choice : config->get_vendor_change_choice();
    auto repo = CoprRepo(base, config, get_project_spec(), opt_chroot);
    if (repo.save_interactive()) {
        ask_and_apply_vendor_change_policy(
            base,
            repo.get_enabled_repoids(),
            repo.get_vendor_policy_name(),
            opt_vendor_change_choice,
            vendor_change_default_choice);
    }
}


}  // namespace dnf5
