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

#include "libdnf5/advisory/advisory.hpp"

#include "solv/pool.hpp"
#include "utils/string.hpp"

#include "libdnf5/advisory/advisory_collection.hpp"
#include "libdnf5/advisory/advisory_reference.hpp"
#include "libdnf5/common/exception.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <fmt/format.h>

namespace libdnf5::advisory {

class Advisory::Impl {
public:
    Impl(const libdnf5::BaseWeakPtr & base, AdvisoryId id) : base(base), id(id) {}

private:
    friend Advisory;

    BaseWeakPtr base;

    AdvisoryId id;
};

Advisory::Advisory(const libdnf5::BaseWeakPtr & base, AdvisoryId id) : p_impl(std::make_unique<Impl>(base, id)) {}

Advisory::Advisory(const Advisory & src) : p_impl(new Impl(*src.p_impl)) {}
Advisory & Advisory::operator=(const Advisory & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}

Advisory::Advisory(Advisory && src) noexcept = default;
Advisory & Advisory::operator=(Advisory && src) noexcept = default;

bool Advisory::operator==(const Advisory & other) const noexcept {
    return p_impl->id == other.p_impl->id && p_impl->base == other.p_impl->base;
}

bool Advisory::operator!=(const Advisory & other) const noexcept {
    return !(*this == other);
}


std::string Advisory::get_name() const {
    const char * name;
    name = get_rpm_pool(p_impl->base).lookup_str(p_impl->id.id, SOLVABLE_NAME);

    if (strncmp(
            libdnf5::solv::SOLVABLE_NAME_ADVISORY_PREFIX, name, libdnf5::solv::SOLVABLE_NAME_ADVISORY_PREFIX_LENGTH) !=
        0) {
        throw RuntimeError(
            M_("Bad libsolv id for advisory \"{}\", solvable name \"{}\" doesn't have advisory prefix \"{}\""),
            p_impl->id.id,
            std::string(name),
            std::string(libdnf5::solv::SOLVABLE_NAME_ADVISORY_PREFIX));
    }

    return std::string(name + libdnf5::solv::SOLVABLE_NAME_ADVISORY_PREFIX_LENGTH);
}

std::string Advisory::get_type() const {
    return std::string(get_rpm_pool(p_impl->base).lookup_str(p_impl->id.id, SOLVABLE_PATCHCATEGORY));
}

std::string Advisory::get_severity() const {
    //TODO(amatej): should we call SolvPrivate::internalize_libsolv_repo(solvable->repo);
    //              before pool.lookup_str?
    //              If so do this just once in solv::advisroy_private
    return libdnf5::utils::string::c_to_str(get_rpm_pool(p_impl->base).lookup_str(p_impl->id.id, UPDATE_SEVERITY));
}

unsigned long long Advisory::get_buildtime() const {
    return get_rpm_pool(p_impl->base).lookup_num(p_impl->id.id, SOLVABLE_BUILDTIME);
}

std::string Advisory::get_title() const {
    // SOLVABLE_SUMMARY is misnamed, it actually stores the title
    return libdnf5::utils::string::c_to_str(get_rpm_pool(p_impl->base).lookup_str(p_impl->id.id, SOLVABLE_SUMMARY));
}

std::string Advisory::get_vendor() const {
    return libdnf5::utils::string::c_to_str(get_rpm_pool(p_impl->base).lookup_str(p_impl->id.id, SOLVABLE_VENDOR));
}

std::string Advisory::get_rights() const {
    return libdnf5::utils::string::c_to_str(get_rpm_pool(p_impl->base).lookup_str(p_impl->id.id, UPDATE_RIGHTS));
}

std::string Advisory::get_status() const {
    return libdnf5::utils::string::c_to_str(get_rpm_pool(p_impl->base).lookup_str(p_impl->id.id, UPDATE_STATUS));
}

std::string Advisory::get_message() const {
    return libdnf5::utils::string::c_to_str(get_rpm_pool(p_impl->base).lookup_str(p_impl->id.id, UPDATE_MESSAGE));
}

std::string Advisory::get_description() const {
    return libdnf5::utils::string::c_to_str(get_rpm_pool(p_impl->base).lookup_str(p_impl->id.id, SOLVABLE_DESCRIPTION));
}

AdvisoryId Advisory::get_id() const {
    return p_impl->id;
}

std::vector<AdvisoryReference> Advisory::get_references(std::vector<std::string> types) const {
    auto & pool = get_rpm_pool(p_impl->base);

    std::vector<AdvisoryReference> output;

    Dataiterator di;
    dataiterator_init(&di, *pool, 0, p_impl->id.id, UPDATE_REFERENCE, 0, 0);

    for (int index = 0; dataiterator_step(&di); index++) {
        dataiterator_setpos(&di);
        std::string current_type = std::string(pool.lookup_str(SOLVID_POS, UPDATE_REFERENCE_TYPE));

        if (types.empty() || std::find(types.begin(), types.end(), current_type) != types.end()) {
            output.emplace_back(AdvisoryReference(p_impl->base, p_impl->id, index));
        }
    }

    dataiterator_free(&di);
    return output;
}

std::vector<AdvisoryCollection> Advisory::get_collections() const {
    std::vector<AdvisoryCollection> output;

    Dataiterator di;
    dataiterator_init(&di, *get_rpm_pool(p_impl->base), 0, p_impl->id.id, UPDATE_COLLECTIONLIST, 0, 0);

    for (int index = 0; dataiterator_step(&di); index++) {
        dataiterator_setpos(&di);
        output.emplace_back(AdvisoryCollection(p_impl->base, p_impl->id, index));
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

}  // namespace libdnf5::advisory
