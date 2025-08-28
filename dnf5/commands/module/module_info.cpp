// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "module_info.hpp"

#include <libdnf5-cli/output/adapters/module.hpp>
#include <libdnf5-cli/output/moduleinfo.hpp>
#include <libdnf5/module/module_query.hpp>

namespace dnf5 {

void ModuleInfoCommand::print(const libdnf5::module::ModuleQuery & query) {
    std::vector<std::unique_ptr<libdnf5::cli::output::IModuleItem>> items;
    items.reserve(query.list().size());
    for (auto & obj : query.list()) {
        items.emplace_back(new libdnf5::cli::output::ModuleItemAdapter(obj));
    }
    libdnf5::cli::output::print_moduleinfo_table(items);
}

void ModuleInfoCommand::print_hint() {
    libdnf5::cli::output::print_moduleinfo_table_hint();
}

}  // namespace dnf5
