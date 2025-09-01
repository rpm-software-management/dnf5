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

#include "module_list.hpp"

#include <libdnf5-cli/output/adapters/module.hpp>
#include <libdnf5-cli/output/modulelist.hpp>
#include <libdnf5/module/module_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void ModuleListCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("List module streams");

    enabled = std::make_unique<ModuleEnabledOption>(*this);
    disabled = std::make_unique<ModuleDisabledOption>(*this);
    module_specs = std::make_unique<ModuleSpecArguments>(*this);
}

void ModuleListCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void ModuleListCommand::run() {
    libdnf5::module::ModuleQuery query(get_context().get_base(), true);
    auto module_specs_str = module_specs->get_value();
    std::set<std::string> unmatched_module_spec;

    if (module_specs_str.size() > 0) {
        for (const auto & module_spec : module_specs_str) {
            bool module_spec_matched = false;
            for (const auto & nsvcap : libdnf5::module::Nsvcap::parse(module_spec)) {
                libdnf5::module::ModuleQuery nsvcap_query(get_context().get_base(), false);
                nsvcap_query.filter_nsvca(nsvcap, libdnf5::sack::QueryCmp::GLOB);
                if (!nsvcap_query.empty()) {
                    query |= nsvcap_query;
                    module_spec_matched = true;
                }
            }
            if (!module_spec_matched) {
                unmatched_module_spec.insert(module_spec);
            }
        }
    } else {
        query = libdnf5::module::ModuleQuery(get_context().get_base(), false);
    }

    if (enabled->get_value()) {
        query.filter_enabled();
    } else if (disabled->get_value()) {
        query.filter_disabled();
    }

    print(query);
    if (!query.empty()) {
        print_hint();
    }

    if (!unmatched_module_spec.empty()) {
        for (auto const & module_spec : unmatched_module_spec) {
            std::cerr << libdnf5::utils::sformat(_("No matches found for \"{}\"."), module_spec) << std::endl;
        }
    }
}

void ModuleListCommand::print(const libdnf5::module::ModuleQuery & query) {
    std::vector<std::unique_ptr<output::IModuleItem>> items;
    items.reserve(query.list().size());
    for (auto & obj : query.list()) {
        items.emplace_back(new output::ModuleItemAdapter(obj));
    }
    libdnf5::cli::output::print_modulelist_table(items);
}

void ModuleListCommand::print_hint() {
    libdnf5::cli::output::print_modulelist_table_hint();
}

}  // namespace dnf5
