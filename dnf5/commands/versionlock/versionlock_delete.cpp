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

#include "versionlock_delete.hpp"

#include <libdnf5/common/sack/query_cmp.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/versionlock_config.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void VersionlockDeleteCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Remove any matching versionlock configuration entries"));

    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto * keys = parser.add_new_positional_arg("specs", ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
    keys->set_description(_("List of package specs to remove versionlock for"));
    keys->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                pkg_specs.emplace_back(argv[i]);
            }
            return true;
        });
    cmd.register_positional_arg(keys);
}

void delete_package(libdnf5::rpm::VersionlockConfig & vl_config, std::string_view spec) {
    auto remove_predicate = [spec](libdnf5::rpm::VersionlockPackage & pkg) {
        if (pkg.is_valid() && pkg.get_name() == spec) {
            std::cout << _("Deleting versionlock entry:") << std::endl;
            std::cout << pkg.to_string(false, true) << std::endl;
            return true;
        }
        return false;
    };

    auto & vl_packages = vl_config.get_packages();
    vl_packages.erase(std::remove_if(vl_packages.begin(), vl_packages.end(), remove_predicate), vl_packages.end());
}

void VersionlockDeleteCommand::run() {
    auto & ctx = get_context();
    auto package_sack = ctx.get_base().get_rpm_package_sack();
    auto vl_config = package_sack->get_versionlock_config();
    auto orig_size = vl_config.get_packages().size();

    for (const auto & spec : pkg_specs) {
        delete_package(vl_config, spec);
    }
    if (vl_config.get_packages().size() != orig_size) {
        vl_config.save();
    }
}

}  // namespace dnf5
