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
#include "copr_repo.hpp"

#include <libdnf5/base/vendor_change_manager.hpp>
#include <libdnf5/conf/const.hpp>

#include <iostream>

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


// Asks the user whether a vendor change to/from packages of the repositories enabled by the 'enable'
// command should be allowed, and writes the corresponding vendor change policy/policies (empty ones
// if not). The default is to not allow a vendor change in either direction.
// With --assumeyes, this default is kept without asking, matching option 1 below.
//
// Within a single vendor change policy, its outgoing_packages and incoming_packages filters are
// ANDed together (both must match for the policy to apply) -- so "allow both directions" cannot be
// expressed as one policy with both an "in:" and an "out:" filter; that would only permit replacing
// a package installed from these repositories with another package also from these repositories.
// Allowing both directions independently therefore requires two separate policy files, prefixed
// "i" and "o" (a prefix, rather than a suffix, cannot collide with a real base filename, since
// those always start with "_copr:").
void ask_and_apply_vendor_change_policy(
    libdnf5::Base & base, const std::vector<std::string> & enabled_repoids, const std::string & base_filename) {
    auto & config = base.get_config();
    const bool assume_yes = config.get_assumeyes_option().get_value();

    int choice;
    std::cerr << std::endl;
    if (assume_yes) {
        std::cerr << _(
            "Not allowing a vendor change in either direction for the repositories enabled "
            "by this command (--assumeyes).\n");
        choice = 1;
    } else {
        std::cerr << _(
            "Packages from the repositories enabled by this command can be allowed to\n"
            "replace, or be replaced by, packages from a different vendor during package\n"
            "replacement (upgrade/downgrade/reinstall) operations:\n");
        std::cerr << _("  1. Do not allow a vendor change in either direction (default)\n");
        std::cerr << _(
            "  2. Allow packages from the repositories enabled by this command to\n"
            "     replace an already installed package from a different vendor\n");
        std::cerr << _(
            "  3. Allow a package installed from the repositories enabled by this\n"
            "     command to be replaced by a package from a different vendor\n");
        std::cerr << _("  4. Allow both of the above (vendor change allowed in both directions)\n");

        choice = 1;
        while (true) {
            std::cerr << _("Enter your choice [1-4, default 1]: ");

            std::string line;
            if (!std::getline(std::cin, line) || line.empty()) {
                choice = 1;
                break;
            }

            try {
                std::size_t pos;
                choice = std::stoi(line, &pos);
                if (pos == line.size() && choice >= 1 && choice <= 4) {
                    break;
                }
            } catch (const std::exception &) {
            }

            std::cerr << _("Invalid choice, please enter a number between 1 and 4.\n");
        }
    }

    const bool allow_incoming = (choice == 2 || choice == 4);
    const bool allow_outgoing = (choice == 3 || choice == 4);

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
}


void CoprEnableCommand::run() {
    auto & base = get_context().get_base();
    std::unique_ptr<dnf5::CoprConfig> config = std::make_unique<dnf5::CoprConfig>(base);
    auto repo = CoprRepo(base, config, get_project_spec(), opt_chroot);
    if (repo.save_interactive()) {
        ask_and_apply_vendor_change_policy(base, repo.get_enabled_repoids(), repo.get_vendor_policy_name());
    }
}


}  // namespace dnf5
