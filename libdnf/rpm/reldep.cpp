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

#include "libdnf/rpm/reldep.hpp"

#include "libdnf/solv/pool.hpp"
#include "libdnf/solv/reldep_parser.hpp"

// workaround, libsolv lacks 'extern "C"' in its header file
extern "C" {
#include <solv/pool.h>
#include <solv/pool_parserpmrichdep.h>
#include <solv/util.h>
}

#include <stdexcept>

namespace libdnf::rpm {


Reldep::Reldep(const BaseWeakPtr & base, ReldepId dependency_id) : base(base), id(dependency_id) {}

Reldep::Reldep(const BaseWeakPtr & base, const char * name, const char * version, CmpType cmp_type) : base(base) {
    id = get_reldep_id(base, name, version, cmp_type);
}

Reldep::Reldep(const BaseWeakPtr & base, const std::string & reldep_string) : base(base) {
    id = get_reldep_id(base, reldep_string);
}

Reldep::Reldep(libdnf::Base & base, const std::string & reldep_string) : Reldep(base.get_weak_ptr(), reldep_string) {}

Reldep::Reldep(Reldep && reldep) : base(std::move(reldep.base)), id(std::move(reldep.id)) {}

const char * Reldep::get_name() const {
    return get_pool(base).id2str(id.id);
}
const char * Reldep::get_relation() const {
    return get_pool(base).id2rel(id.id);
}
const char * Reldep::get_version() const {
    return get_pool(base).id2evr(id.id);
}
std::string Reldep::to_string() {
    auto * cstring = get_pool(base).dep2str(id.id);
    return cstring ? std::string(cstring) : std::string();
}

ReldepId Reldep::get_reldep_id(const BaseWeakPtr & base, const char * name, const char * version, CmpType cmp_type, int create) {
    static_assert(
        static_cast<int>(Reldep::CmpType::EQ) == REL_EQ, "Reldep::ComparisonType::EQ is not identical to solv/REL_EQ");
    static_assert(
        static_cast<int>(Reldep::CmpType::LT) == REL_LT, "Reldep::ComparisonType::LT is not identical to solv/REL_LT");
    static_assert(
        static_cast<int>(Reldep::CmpType::GT) == REL_GT, "Reldep::ComparisonType::GT is not identical to solv/REL_GT");
    auto & pool = get_pool(base);
    Id id = pool.str2id(name, create);
    if (id == 0) {
        return ReldepId();
    }

    if (version) {
        Id evr_id = pool.str2id(version, 1);
        id = pool.rel2id(id, evr_id, static_cast<int>(cmp_type), true);
    }
    return ReldepId(id);
}

ReldepId Reldep::get_reldep_id(const BaseWeakPtr & base, const std::string & reldep_str, int create) {
    if (reldep_str[0] == '(') {
        // Rich dependency
        Id id = pool_parserpmrichdep(*get_pool(base), reldep_str.c_str());
        // TODO(jmracek) Replace runtime_error. Do we need to throw an error?
        if (id == 0) {
            throw RuntimeError(M_("Cannot parse a dependency string"));
        }
        return ReldepId(id);
    }

    libdnf::solv::ReldepParser dep_splitter;
    if (!dep_splitter.parse(reldep_str)) {
        throw RuntimeError(M_("Cannot parse a dependency string"));
    }
    return get_reldep_id(
        base, dep_splitter.get_name_cstr(), dep_splitter.get_evr_cstr(), dep_splitter.get_cmp_type(), create);
}

}  // namespace libdnf::rpm
