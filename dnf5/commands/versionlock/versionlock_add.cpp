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

#include "versionlock_add.hpp"

#include "utils.hpp"

#include <libdnf5/common/sack/query_cmp.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/versionlock_config.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void VersionlockAddCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Add new entry to versionlock configuration"));

    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto * keys =
        parser.add_new_positional_arg("package-spec-N", ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
    keys->set_description(_("List of package package-spec-N to add versionlock for"));
    keys->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                pkg_specs.emplace_back(argv[i]);
            }
            return true;
        });
    cmd.register_positional_arg(keys);
}

void VersionlockAddCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

bool lock_version(
    libdnf5::rpm::VersionlockConfig & vl_config, const libdnf5::rpm::Package & pkg, const std::string & comment) {
    auto evr = pkg.get_evr();
    auto name = pkg.get_name();
    auto & vl_packages = vl_config.get_packages();
    // check whether a rule for this version is already present
    for (const auto & vl_package : vl_packages) {
        if (!vl_package.is_valid() || vl_package.get_name() != name) {
            continue;
        }
        for (const auto & vl_cond : vl_package.get_conditions()) {
            if (!vl_cond.is_valid()) {
                continue;
            }
            if (vl_cond.get_key() == libdnf5::rpm::VersionlockCondition::Keys::EVR &&
                vl_cond.get_comparator() == libdnf5::sack::QueryCmp::EQ && vl_cond.get_value() == evr) {
                // do not add duplicite versionlock rules
                return false;
            }
        }
    }

    const libdnf5::rpm::VersionlockCondition vl_condition("evr", "=", evr);
    libdnf5::rpm::VersionlockPackage vl_package(name, std::vector<libdnf5::rpm::VersionlockCondition>{vl_condition});
    vl_package.set_comment(comment);
    vl_packages.emplace_back(std::move(vl_package));
    return true;
}

void VersionlockAddCommand::run() {
    auto & ctx = get_context();
    auto package_sack = ctx.get_base().get_rpm_package_sack();
    auto vl_config = package_sack->get_versionlock_config();
    auto orig_size = vl_config.get_packages().size();

    const auto comment = format_comment("add");

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
        // The spec can resolve to multiple package names. Handle each name in
        // the query separately.
        std::set<std::string> package_names;
        for (const auto & pkg : query) {
            package_names.emplace(pkg.get_name());
        }
        for (const auto & name : package_names) {
            libdnf5::rpm::PackageQuery name_query(query);
            name_query.filter_name(name);
            libdnf5::rpm::PackageQuery installed_query(name_query);
            installed_query.filter_installed();
            if (!installed_query.empty()) {
                // if spec is installed, add only installed version
                name_query = installed_query;
            }

            std::unordered_set<std::string> versions{};
            for (const auto & pkg : name_query) {
                auto evr = pkg.get_evr();
                if (versions.contains(evr)) {
                    continue;
                }
                versions.emplace(evr);
                if (lock_version(vl_config, pkg, comment)) {
                    std::cout << libdnf5::utils::sformat(_("Adding versionlock on \"{0} = {1}\"."), pkg.get_name(), evr)
                              << std::endl;
                } else {
                    std::cerr << libdnf5::utils::sformat(_("Package \"{}\" is already locked."), spec) << std::endl;
                }
            }
        }
    }
    if (vl_config.get_packages().size() != orig_size) {
        vl_config.save();
    }
}

}  // namespace dnf5
