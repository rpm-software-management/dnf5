// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
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

#include "libdnf5/rpm/reldep.hpp"

#include "solv/pool.hpp"
#include "solv/reldep_parser.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

// workaround, libsolv lacks 'extern "C"' in its header file
extern "C" {
#include <solv/pool.h>
#include <solv/pool_parserpmrichdep.h>
#include <solv/util.h>
}

namespace libdnf5::rpm {

class Reldep::Impl {
public:
    Impl(const BaseWeakPtr & base, ReldepId dep_id) : base(base), id(dep_id) {}

private:
    friend Reldep;

    BaseWeakPtr base;
    ReldepId id;
};

Reldep::Reldep(const BaseWeakPtr & base, ReldepId dependency_id) : p_impl(new Impl(base, dependency_id)) {}

Reldep::Reldep(const BaseWeakPtr & base, const char * name, const char * version, CmpType cmp_type)
    : p_impl(new Impl(base, get_reldep_id(base, name, version, cmp_type))) {}

Reldep::Reldep(const BaseWeakPtr & base, const std::string & reldep_string)
    : p_impl(new Impl(base, get_reldep_id(base, reldep_string))) {}

Reldep::~Reldep() = default;

Reldep::Reldep(libdnf5::Base & base, const std::string & reldep_string) : Reldep(base.get_weak_ptr(), reldep_string) {}

Reldep::Reldep(const Reldep & reldep) = default;
Reldep::Reldep(Reldep && reldep) noexcept = default;

Reldep & Reldep::operator=(const Reldep & other) = default;

bool Reldep::operator==(const Reldep & other) const noexcept {
    return p_impl->id == other.p_impl->id && p_impl->base == other.p_impl->base;
}

bool Reldep::operator!=(const Reldep & other) const noexcept {
    return p_impl->id != other.p_impl->id || p_impl->base != other.p_impl->base;
}

const char * Reldep::get_name() const {
    return get_rpm_pool(p_impl->base).id2str(p_impl->id.id);
}
const char * Reldep::get_relation() const {
    return get_rpm_pool(p_impl->base).id2rel(p_impl->id.id);
}
const char * Reldep::get_version() const {
    return get_rpm_pool(p_impl->base).id2evr(p_impl->id.id);
}
std::string Reldep::to_string() const {
    auto * cstring = get_rpm_pool(p_impl->base).dep2str(p_impl->id.id);
    return cstring ? std::string(cstring) : std::string();
}

std::string Reldep::to_string_description() const {
    return fmt::format("<libdnf5.rpm.Reldep object, {}, id: {}>", to_string(), get_id().id);
}

ReldepId Reldep::get_reldep_id(
    const BaseWeakPtr & base, const char * name, const char * version, CmpType cmp_type, int create) {
    static_assert(
        static_cast<int>(Reldep::CmpType::EQ) == REL_EQ, "Reldep::ComparisonType::EQ is not identical to solv/REL_EQ");
    static_assert(
        static_cast<int>(Reldep::CmpType::LT) == REL_LT, "Reldep::ComparisonType::LT is not identical to solv/REL_LT");
    static_assert(
        static_cast<int>(Reldep::CmpType::GT) == REL_GT, "Reldep::ComparisonType::GT is not identical to solv/REL_GT");
    auto & pool = get_rpm_pool(base);
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
    if (is_rich_dependency(reldep_str)) {
        Id id = pool_parserpmrichdep(*get_rpm_pool(base), reldep_str.c_str());
        // TODO(jmracek) Replace runtime_error. Do we need to throw an error?
        if (id == 0) {
            throw RuntimeError(M_("Cannot parse a dependency string"));
        }
        return ReldepId(id);
    }

    libdnf5::solv::ReldepParser dep_splitter;
    if (!dep_splitter.parse(reldep_str)) {
        throw RuntimeError(M_("Cannot parse a dependency string"));
    }
    return get_reldep_id(
        base, dep_splitter.get_name_cstr(), dep_splitter.get_evr_cstr(), dep_splitter.get_cmp_type(), create);
}

ReldepId Reldep::get_id() const noexcept {
    return p_impl->id;
};

/// Return weak pointer to base
BaseWeakPtr Reldep::get_base() const {
    return p_impl->base;
};

bool Reldep::is_rich_dependency(const std::string & pattern) {
    return pattern[0] == '(';
};

/// Return unique ID representing Reldep
int Reldep::get_hash() const {
    return get_id().id;
};

}  // namespace libdnf5::rpm
