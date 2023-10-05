/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "module_list.hpp"

#include <libdnf5-cli/output/modulelist.hpp>
#include <libdnf5/module/module_query.hpp>

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
    libdnf5::module::ModuleQuery query(get_context().base, true);
    auto module_specs_str = module_specs->get_value();
    if (module_specs_str.size() > 0) {
        for (const auto & module_spec : module_specs_str) {
            for (const auto & nsvcap : libdnf5::module::Nsvcap::parse(module_spec)) {
                libdnf5::module::ModuleQuery nsvcap_query(get_context().base, false);
                nsvcap_query.filter_nsvca(nsvcap, libdnf5::sack::QueryCmp::GLOB);
                if (!nsvcap_query.empty()) {
                    query |= nsvcap_query;
                }
            }
        }
    } else {
        query = libdnf5::module::ModuleQuery(get_context().base, false);
    }

    if (enabled->get_value()) {
        query.filter_enabled();
    } else if (disabled->get_value()) {
        query.filter_disabled();
    }

    output::print_modulelist_table(query.list());
    output::print_modulelist_table_hint();
}

}  // namespace dnf5
