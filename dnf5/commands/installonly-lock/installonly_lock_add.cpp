// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "installonly_lock_add.hpp"

#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void InstallonlyLockAddCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Lock installed installonly packages against automatic removal"));

    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto * keys = parser.add_new_positional_arg(
        "package-spec-NPFB", ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
    keys->set_description(_("List of installed installonly packages to lock"));
    keys->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                pkg_specs.emplace_back(argv[i]);
            }
            return true;
        });
    keys->set_complete_hook_func([&ctx](const char * arg) { return ctx.match_specs(arg, true, false, false, false); });
    cmd.register_positional_arg(keys);
}

void InstallonlyLockAddCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
}

void InstallonlyLockAddCommand::run() {
    auto & ctx = get_context();

    libdnf5::ResolveSpecSettings settings;
    settings.set_with_nevra(true);
    settings.set_with_provides(false);
    settings.set_with_filenames(false);
    settings.set_with_binaries(false);

    for (const auto & spec : pkg_specs) {
        libdnf5::rpm::PackageQuery query(ctx.get_base());
        query.filter_installed();
        query.filter_installonly();
        query.resolve_pkg_spec(spec, settings, false);
        if (query.empty()) {
            std::cerr << libdnf5::utils::sformat(_("No installed installonly package found for \"{}\"."), spec)
                      << std::endl;
            continue;
        }
        for (auto pkg : query) {
            if (pkg.is_installonly_locked()) {
                std::cerr << libdnf5::utils::sformat(_("Package \"{}\" is already locked."), pkg.get_nevra())
                          << std::endl;
                continue;
            }
            pkg.set_installonly_locked(true);
            std::cout << libdnf5::utils::sformat(_("Adding installonly lock on \"{}\"."), pkg.get_nevra()) << std::endl;
        }
    }
}

}  // namespace dnf5
