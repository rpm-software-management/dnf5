/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "libdnf5/module/module_query.hpp"

#include "module/module_sack_impl.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/module/module_item.hpp"
#include "libdnf5/module/module_sack.hpp"
#include "libdnf5/module/nsvcap.hpp"
#include "libdnf5/utils/patterns.hpp"

extern "C" {
#include <solv/pool.h>
}


namespace libdnf5::module {


ModuleQuery::ModuleQuery(const BaseWeakPtr & base, bool empty) : base(base) {
    if (empty) {
        return;
    }

    // Copy all repos from ModuleSack to the this object
    auto sack = base->get_module_sack();
    for (auto & it : sack->get_modules()) {
        add(*it.get());
    }
}


ModuleQuery::ModuleQuery(libdnf5::Base & base, bool empty) : ModuleQuery(base.get_weak_ptr(), empty) {}


void ModuleQuery::filter_name(const std::string & pattern, libdnf5::sack::QueryCmp cmp) {
    filter(Get::name, pattern, cmp);
}


void ModuleQuery::filter_name(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp) {
    filter(Get::name, patterns, cmp);
}


void ModuleQuery::filter_stream(const std::string & pattern, libdnf5::sack::QueryCmp cmp) {
    filter(Get::stream, pattern, cmp);
}


void ModuleQuery::filter_stream(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp) {
    filter(Get::stream, patterns, cmp);
}


void ModuleQuery::filter_version(const std::string & pattern, libdnf5::sack::QueryCmp cmp) {
    filter(Get::version, pattern, cmp);
}


void ModuleQuery::filter_version(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp) {
    filter(Get::version, patterns, cmp);
}


void ModuleQuery::filter_context(const std::string & pattern, libdnf5::sack::QueryCmp cmp) {
    filter(Get::context, pattern, cmp);
}


void ModuleQuery::filter_context(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp) {
    filter(Get::context, patterns, cmp);
}


void ModuleQuery::filter_arch(const std::string & pattern, libdnf5::sack::QueryCmp cmp) {
    filter(Get::arch, pattern, cmp);
}


void ModuleQuery::filter_arch(const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp) {
    filter(Get::arch, patterns, cmp);
}


bool ModuleQuery::latest_cmp(const ModuleItem * module_item_1, const ModuleItem * module_item_2) {
    Pool * pool = module_item_1->get_module_sack()->p_impl->pool;
    const Solvable * s1 = pool_id2solvable(pool, module_item_1->get_id().id);
    const Solvable * s2 = pool_id2solvable(pool, module_item_2->get_id().id);
    if (s1->name != s2->name) {
        return s1->name < s2->name;
    }
    if (s1->arch != s2->arch) {
        return s1->arch < s2->arch;
    }
    return module_item_1->get_version() > module_item_2->get_version();
}


void ModuleQuery::filter_latest(int limit) {
    if (limit == 0) {
        clear();
        return;
    }

    std::vector<const ModuleItem *> same_nsca_vector;
    same_nsca_vector.reserve(size());

    for (auto & module_item : get_data()) {
        same_nsca_vector.push_back(&module_item);
    }
    if (limit > 0) {
        sort(same_nsca_vector.begin(), same_nsca_vector.end(), latest_cmp);
    } else {
        sort(same_nsca_vector.rbegin(), same_nsca_vector.rend(), latest_cmp);
        limit *= -1;
    }

    std::string last_nsca = same_nsca_vector.front()->get_name_stream_staticcontext_arch();
    long long last_version = -1;  // invalid version value that cannot be in a module item
    int kept_in_query = 0;
    for (auto module_item : same_nsca_vector) {
        // If the nsca is different from the last, it means a new block, so start a new `kept_in_query` count and set version to invalid value again.
        std::string nsca = module_item->get_name_stream_staticcontext_arch();
        if (last_nsca != nsca) {
            last_nsca = nsca;
            kept_in_query = 0;
            last_version = -1;
        }

        long long version = module_item->get_version();
        if (last_version == version) {
            // Do nothing => keep it in query, do not increase `kept_in_query` count.
            // It cannot happen that we would go over the limit, because the `last_version` can be only -1 or a version of an item that was kept in the query.
        } else if (kept_in_query < limit) {
            last_version = version;
            kept_in_query += 1;
        } else {
            get_data().erase(*module_item);
        }
    }
}


void ModuleQuery::filter_nsvca(const Nsvcap & nsvcap, libdnf5::sack::QueryCmp cmp) {
    const std::string & name = nsvcap.get_name();
    if (!name.empty()) {
        filter_name(name, cmp);
    }
    const std::string & stream = nsvcap.get_stream();
    if (!stream.empty()) {
        filter_stream(stream, cmp);
    }
    const std::string & version = nsvcap.get_version();
    if (!version.empty()) {
        filter_version(version, cmp);
    }
    const std::string & context = nsvcap.get_context();
    if (!context.empty()) {
        filter_context(context, cmp);
    }
    const std::string & arch = nsvcap.get_arch();
    if (!arch.empty()) {
        filter_arch(arch, cmp);
    }
}


void ModuleQuery::filter_enabled() {
    filter(Get::is_enabled, true, libdnf5::sack::QueryCmp::EQ);
}


void ModuleQuery::filter_disabled() {
    filter(Get::is_disabled, true, libdnf5::sack::QueryCmp::EQ);
}


std::pair<bool, Nsvcap> ModuleQuery::resolve_module_spec(const std::string & module_spec) {
    libdnf5::sack::QueryCmp cmp = libdnf5::utils::is_glob_pattern(module_spec.c_str()) ? libdnf5::sack::QueryCmp::GLOB
                                                                                       : libdnf5::sack::QueryCmp::EQ;

    std::vector<Nsvcap> possible_nsvcaps = Nsvcap::parse(module_spec);
    for (Nsvcap & nsvcap : possible_nsvcaps) {
        ModuleQuery module_query(*this);
        module_query.filter_nsvca(nsvcap, cmp);
        if (!module_query.empty()) {
            *this = std::move(module_query);
            return {true, Nsvcap(nsvcap)};
        }
    }

    clear();
    return {false, Nsvcap()};
}


bool ModuleQuery::Get::is_enabled(const ModuleItem & obj) {
    return obj.get_status() == ModuleStatus::ENABLED;
}


bool ModuleQuery::Get::is_disabled(const ModuleItem & obj) {
    return obj.get_status() == ModuleStatus::DISABLED;
}


}  // namespace libdnf5::module
