/*
Copyright (C) 2021 Red Hat, Inc.

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

#include "libdnf/advisory/advisory.hpp"

#include "libdnf/advisory/advisory_collection.hpp"
#include "libdnf/advisory/advisory_reference.hpp"
#include "libdnf/common/exception.hpp"
#include "libdnf/rpm/package_sack_impl.hpp"
#include "libdnf/utils/utils_internal.hpp"

#include <fmt/format.h>
#include <solv/pool.h>

namespace libdnf::advisory {

Advisory::Advisory(const libdnf::rpm::PackageSackWeakPtr & sack, AdvisoryId id) : id(id), sack(sack) {}

std::string Advisory::get_name() const {
    const char * name;
    Pool * pool = sack->p_impl->get_pool();

    name = pool_lookup_str(pool, id.id, SOLVABLE_NAME);

    if (strncmp(
            libdnf::utils::SOLVABLE_NAME_ADVISORY_PREFIX, name, libdnf::utils::SOLVABLE_NAME_ADVISORY_PREFIX_LENGTH) !=
        0) {
        auto msg = fmt::format(
            R"**(Bad libsolv id for advisory "{}", solvable name "{}" doesn't have advisory prefix "{}")**",
            id.id,
            name,
            libdnf::utils::SOLVABLE_NAME_ADVISORY_PREFIX);
        throw RuntimeError(msg);
    }

    return std::string(name + libdnf::utils::SOLVABLE_NAME_ADVISORY_PREFIX_LENGTH);
}

Advisory::Type Advisory::get_type() const {
    const char * type;
    Pool * pool = sack->p_impl->get_pool();
    type = pool_lookup_str(pool, id.id, SOLVABLE_PATCHCATEGORY);

    if (type == NULL) {
        return Advisory::Type::UNKNOWN;
    }
    if (!strcmp(type, "bugfix")) {
        return Advisory::Type::BUGFIX;
    }
    if (!strcmp(type, "enhancement")) {
        return Advisory::Type::ENHANCEMENT;
    }
    if (!strcmp(type, "security")) {
        return Advisory::Type::SECURITY;
    }
    if (!strcmp(type, "newpackage")) {
        return Advisory::Type::NEWPACKAGE;
    }

    return Advisory::Type::UNKNOWN;
}

const char * Advisory::get_type_cstring() const {
    Pool * pool = sack->p_impl->get_pool();
    return pool_lookup_str(pool, id.id, SOLVABLE_PATCHCATEGORY);
}

const char * Advisory::advisory_type_to_cstring(Type type) {
    switch (type) {
        case Type::ENHANCEMENT:
            return "enhancement";
        case Type::SECURITY:
            return "security";
        case Type::NEWPACKAGE:
            return "newpackage";
        case Type::BUGFIX:
            return "bugfix";
        default:
            return NULL;
    }
}

std::string Advisory::get_severity() const {
    Pool * pool = sack->p_impl->get_pool();

    //TODO(amatej): should we call SolvPrivate::internalize_libsolv_repo(solvable->repo);
    //              before pool_lookup_str?
    //              If so do this just once in solv::advisroy_private
    const char * severity = pool_lookup_str(pool, id.id, UPDATE_SEVERITY);
    return severity ? std::string(severity) : std::string();
}

AdvisoryId Advisory::get_id() const {
    return id;
}

std::vector<AdvisoryReference> Advisory::get_references(AdvisoryReferenceType ref_type) const {
    Pool * pool = sack->p_impl->get_pool();

    std::vector<AdvisoryReference> output;

    Dataiterator di;
    dataiterator_init(&di, pool, 0, id.id, UPDATE_REFERENCE, 0, 0);

    for (int index = 0; dataiterator_step(&di); index++) {
        dataiterator_setpos(&di);
        const char * current_type = pool_lookup_str(pool, SOLVID_POS, UPDATE_REFERENCE_TYPE);

        if (((ref_type & AdvisoryReferenceType::CVE) == AdvisoryReferenceType::CVE &&
             (strcmp(current_type, "cve") == 0)) ||
            ((ref_type & AdvisoryReferenceType::BUGZILLA) == AdvisoryReferenceType::BUGZILLA &&
             (strcmp(current_type, "bugzilla") == 0)) ||
            ((ref_type & AdvisoryReferenceType::VENDOR) == AdvisoryReferenceType::VENDOR &&
             (strcmp(current_type, "vendor") == 0))) {
            output.emplace_back(AdvisoryReference(sack, id, index));
        }
    }

    dataiterator_free(&di);
    return output;
}

std::vector<AdvisoryCollection> Advisory::get_collections() const {
    Pool * pool = sack->p_impl->get_pool();

    std::vector<AdvisoryCollection> output;

    Dataiterator di;
    dataiterator_init(&di, pool, 0, id.id, UPDATE_COLLECTIONLIST, 0, 0);

    for (int index = 0; dataiterator_step(&di); index++) {
        dataiterator_setpos(&di);
        output.emplace_back(AdvisoryCollection(sack, id, index));
    }

    dataiterator_free(&di);

    return output;
}

//TODO(amatej): this could be possibly removed?
bool Advisory::is_applicable() const {
    for (const auto & collection : get_collections()) {
        if (collection.is_applicable()) {
            return true;
        }
    }

    return false;
}

Advisory::~Advisory() = default;

}  // namespace libdnf::advisory
