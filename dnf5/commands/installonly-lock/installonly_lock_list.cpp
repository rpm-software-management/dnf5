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

#include "installonly_lock_list.hpp"

#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>

#include <algorithm>
#include <iostream>
#include <vector>

namespace dnf5 {

using namespace libdnf5::cli;

void InstallonlyLockListCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("List installonly packages locked against automatic removal"));
}

void InstallonlyLockListCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
}

void InstallonlyLockListCommand::run() {
    auto & ctx = get_context();

    libdnf5::rpm::PackageQuery query(ctx.get_base());
    query.filter_installed();
    query.filter_installonly();

    std::vector<std::string> locked_nevras;
    for (const auto & pkg : query) {
        if (pkg.is_installonly_locked()) {
            locked_nevras.push_back(pkg.get_full_nevra());
        }
    }
    std::sort(locked_nevras.begin(), locked_nevras.end());

    for (const auto & nevra : locked_nevras) {
        std::cout << nevra << std::endl;
    }
}

}  // namespace dnf5
