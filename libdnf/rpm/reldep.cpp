/*
Copyright (C) 2018-2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "libdnf/rpm/reldep.hpp"

#include "sack_impl.hpp"
#include "solv/reldep_parser.hpp"

// workaround, libsolv lacks 'extern "C"' in its header file
extern "C" {
#include <solv/pool.h>
#include <solv/pool_parserpmrichdep.h>
#include <solv/util.h>
}

#include <stdexcept>

namespace libdnf::rpm {


Reldep::Reldep(Sack * sack, ReldepId id) : sack(sack), id(id) {}

Reldep::Reldep(Sack * sack, const char * name, const char * version, CmpType cmp_type) : sack(sack) {
    id = get_reldep_id(sack, name, version, cmp_type);
}

Reldep::Reldep(Sack * sack, const std::string & reldep_string) : sack(sack) {
    id = get_reldep_id(sack, reldep_string);
}


Reldep::Reldep(const Reldep & reldep) : sack(reldep.sack), id(reldep.id) {}

Reldep::Reldep(const Reldep && reldep) noexcept : sack(reldep.sack), id(reldep.id) {}

Reldep::~Reldep() = default;
const char * Reldep::get_name() const {
    return pool_id2str(sack->pImpl->pool, id.id);
}
const char * Reldep::get_relation() const {
    return pool_id2rel(sack->pImpl->pool, id.id);
}
const char * Reldep::get_version() const {
    return pool_id2evr(sack->pImpl->pool, id.id);
}
const char * Reldep::to_string() const {
    return pool_dep2str(sack->pImpl->pool, id.id);
}

ReldepId Reldep::get_reldep_id(Sack * sack, const char * name, const char * version, CmpType cmp_type) {
    static_assert(
        static_cast<int>(Reldep::CmpType::EQ) == REL_EQ, "Reldep::ComparisonType::EQ is not identical to solv/REL_EQ");
    static_assert(
        static_cast<int>(Reldep::CmpType::LT) == REL_LT, "Reldep::ComparisonType::LT is not identical to solv/REL_LT");
    static_assert(
        static_cast<int>(Reldep::CmpType::GT) == REL_GT, "Reldep::ComparisonType::GT is not identical to solv/REL_GT");
    Pool * pool = sack->pImpl->pool;
    Id id = pool_str2id(pool, name, 1);

    if (version) {
        Id evr_id = pool_str2id(pool, version, 1);
        id = pool_rel2id(pool, id, evr_id, static_cast<int>(cmp_type), 1);
    }
    return ReldepId(id);
}

ReldepId Reldep::get_reldep_id(Sack * sack, const std::string & reldep_str) {
    if (reldep_str[0] == '(') {
        // Rich dependency
        Pool * pool = sack->pImpl->pool;
        Id id = pool_parserpmrichdep(pool, reldep_str.c_str());
        // TODO(jmracek) Replace runtime_error. Do we need to throw an error?
        if (!id)
            throw std::runtime_error("Cannot parse a dependency string");
        return ReldepId(id);
    } else {
        solv::ReldepParser dep_splitter;
        if (!dep_splitter.parse(reldep_str))
            throw std::runtime_error("Cannot parse a dependency string");
        return get_reldep_id(
            sack, dep_splitter.get_name_cstr(), dep_splitter.get_evr_cstr(), dep_splitter.get_cmp_type());
    }
}

}  // namespace libdnf::rpm
