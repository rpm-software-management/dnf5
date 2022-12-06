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


#include "libdnf/module/module_query.hpp"

#include "module/module_sack_impl.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/module/module_item.hpp"
#include "libdnf/module/module_sack.hpp"

extern "C" {
#include <solv/pool.h>
}


namespace libdnf::module {


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


ModuleQuery::ModuleQuery(libdnf::Base & base, bool empty) : ModuleQuery(base.get_weak_ptr(), empty) {}


void ModuleQuery::filter_name(const std::string & pattern, libdnf::sack::QueryCmp cmp) {
    filter(Get::name, pattern, cmp);
}


void ModuleQuery::filter_name(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp) {
    filter(Get::name, patterns, cmp);
}


void ModuleQuery::filter_stream(const std::string & pattern, libdnf::sack::QueryCmp cmp) {
    filter(Get::stream, pattern, cmp);
}


void ModuleQuery::filter_stream(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp) {
    filter(Get::stream, patterns, cmp);
}


void ModuleQuery::filter_version(const std::string & pattern, libdnf::sack::QueryCmp cmp) {
    filter(Get::version, pattern, cmp);
}


void ModuleQuery::filter_version(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp) {
    filter(Get::version, patterns, cmp);
}


void ModuleQuery::filter_context(const std::string & pattern, libdnf::sack::QueryCmp cmp) {
    filter(Get::context, pattern, cmp);
}


void ModuleQuery::filter_context(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp) {
    filter(Get::context, patterns, cmp);
}


void ModuleQuery::filter_arch(const std::string & pattern, libdnf::sack::QueryCmp cmp) {
    filter(Get::arch, pattern, cmp);
}


void ModuleQuery::filter_arch(const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp) {
    filter(Get::arch, patterns, cmp);
}


}  // namespace libdnf::module
