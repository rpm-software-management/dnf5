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

#include "versionlock_exclude.hpp"

#include "utils.hpp"

#include <libdnf5/common/sack/query_cmp.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/versionlock_config.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void VersionlockExcludeCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Add new exclude entry to versionlock configuration"));

    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto * keys =
        parser.add_new_positional_arg("package-spec-N", ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
    keys->set_description(
        _("List of packages to exclude. If no version is specified, all available versions of the package will be "
          "excluded."));
    keys->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                pkg_specs.emplace_back(argv[i]);
            }
            return true;
        });
    cmd.register_positional_arg(keys);
}

void VersionlockExcludeCommand::configure() {
    auto & context = get_context();
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

bool exclude_versions(
    libdnf5::rpm::VersionlockConfig & vl_config,
    std::string_view name,
    const std::vector<libdnf5::rpm::Package> & packages,
    const std::string & comment) {
    auto & vl_packages = vl_config.get_packages();
    // find entry with same name and only != operators
    for (auto & vl_package : vl_packages) {
        if (!vl_package.is_valid() || vl_package.get_name() != name) {
            continue;
        }
        auto vl_conditions = vl_package.get_conditions();
        if (std::any_of(vl_conditions.begin(), vl_conditions.end(), [](const auto & vl_cond) {
                return vl_cond.get_key() != libdnf5::rpm::VersionlockCondition::Keys::EVR ||
                       vl_cond.get_comparator() != libdnf5::sack::QueryCmp::NEQ;
            })) {
            continue;
        }
        // add missing versions and return
        bool changed = false;
        for (const auto & pkg : packages) {
            const auto evr = pkg.get_evr();
            if (std::any_of(vl_conditions.begin(), vl_conditions.end(), [&evr](const auto & vl_cond) {
                    return vl_cond.get_value() == evr;
                })) {
                // condition for this evr is already present
                continue;
            }
            vl_package.add_condition(libdnf5::rpm::VersionlockCondition{"evr", "!=", evr});
            std::cout << libdnf5::utils::sformat(_("Adding versionlock exclude on \"{0} = {1}\"."), name, evr)
                      << std::endl;
            changed = true;
        }
        return changed;
    }

    // add new entry with all versions and return
    std::vector<libdnf5::rpm::VersionlockCondition> conditions;
    for (const auto & pkg : packages) {
        const auto evr = pkg.get_evr();
        conditions.emplace_back("evr", "!=", evr);
        std::cout << libdnf5::utils::sformat(_("Adding versionlock exclude on \"{0} = {1}\"."), name, evr) << std::endl;
    }
    libdnf5::rpm::VersionlockPackage vl_package(name, std::move(conditions));
    vl_package.set_comment(comment);
    vl_packages.emplace_back(std::move(vl_package));
    return true;
}

void VersionlockExcludeCommand::run() {
    auto & ctx = get_context();
    auto package_sack = ctx.get_base().get_rpm_package_sack();
    auto vl_config = package_sack->get_versionlock_config();

    const auto comment = format_comment("exclude");

    bool changed{false};
    libdnf5::ResolveSpecSettings settings;
    settings.set_with_nevra(true);
    settings.set_with_provides(false);
    settings.set_with_filenames(false);
    settings.set_with_binaries(false);
    for (const auto & spec : pkg_specs) {
        libdnf5::rpm::PackageQuery query(ctx.get_base(), libdnf5::sack::ExcludeFlags::IGNORE_VERSIONLOCK);
        query.resolve_pkg_spec(spec, settings, false);
        if (query.empty()) {
            std::cerr << libdnf5::utils::sformat(_("No package found for \"{}\"."), spec) << std::endl;
            continue;
        }

        // group packages by name
        std::unordered_map<std::string, std::vector<libdnf5::rpm::Package>> versions{};
        for (const auto & pkg : query) {
            versions[pkg.get_name()].emplace_back(pkg);
        }

        for (const auto & [name, packages] : versions) {
            if (exclude_versions(vl_config, name, packages, comment)) {
                changed = true;
            } else {
                std::cerr << libdnf5::utils::sformat(_("Package \"{}\" is already excluded."), spec) << std::endl;
            }
        }
    }
    if (changed) {
        vl_config.save();
    }
}

}  // namespace dnf5
