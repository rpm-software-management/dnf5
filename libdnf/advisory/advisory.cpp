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

#include "libdnf/advisory/advisory.hpp"

#include "libdnf/advisory/advisory_collection.hpp"
#include "libdnf/advisory/advisory_reference.hpp"
#include "libdnf/common/exception.hpp"
#include "libdnf/solv/pool.hpp"
#include "libdnf/utils/string.hpp"

#include <fmt/format.h>

namespace libdnf::advisory {

Advisory::Advisory(const libdnf::BaseWeakPtr & base, AdvisoryId id) : base(base), id(id) {}

Advisory::Advisory(libdnf::Base & base, AdvisoryId id) : Advisory(base.get_weak_ptr(), id) {}

std::string Advisory::get_name() const {
    const char * name;
    name = get_pool(base).lookup_str(id.id, SOLVABLE_NAME);

    if (strncmp(
            libdnf::solv::SOLVABLE_NAME_ADVISORY_PREFIX, name, libdnf::solv::SOLVABLE_NAME_ADVISORY_PREFIX_LENGTH) !=
        0) {
        auto msg = fmt::format(
            R"**(Bad libsolv id for advisory "{}", solvable name "{}" doesn't have advisory prefix "{}")**",
            id.id,
            name,
            libdnf::solv::SOLVABLE_NAME_ADVISORY_PREFIX);
        throw RuntimeError(msg);
    }

    return std::string(name + libdnf::solv::SOLVABLE_NAME_ADVISORY_PREFIX_LENGTH);
}

std::string Advisory::get_type() const {
    return std::string(get_pool(base).lookup_str(id.id, SOLVABLE_PATCHCATEGORY));
}

std::string Advisory::get_severity() const {
    //TODO(amatej): should we call SolvPrivate::internalize_libsolv_repo(solvable->repo);
    //              before pool.lookup_str?
    //              If so do this just once in solv::advisroy_private
    return libdnf::utils::string::c_to_str(get_pool(base).lookup_str(id.id, UPDATE_SEVERITY));
}

unsigned long long Advisory::get_buildtime() const {
    return get_pool(base).lookup_num(id.id, SOLVABLE_BUILDTIME);
}

std::string Advisory::get_title() const {
    // SOLVABLE_SUMMARY is misnamed, it actually stores the title
    return libdnf::utils::string::c_to_str(get_pool(base).lookup_str(id.id, SOLVABLE_SUMMARY));
}

std::string Advisory::get_vendor() const {
    return libdnf::utils::string::c_to_str(get_pool(base).lookup_str(id.id, SOLVABLE_VENDOR));
}

std::string Advisory::get_rights() const {
    return libdnf::utils::string::c_to_str(get_pool(base).lookup_str(id.id, UPDATE_RIGHTS));
}

std::string Advisory::get_status() const {
    return libdnf::utils::string::c_to_str(get_pool(base).lookup_str(id.id, UPDATE_STATUS));
}

std::string Advisory::get_message() const {
    return libdnf::utils::string::c_to_str(get_pool(base).lookup_str(id.id, UPDATE_MESSAGE));
}

std::string Advisory::get_description() const {
    return libdnf::utils::string::c_to_str(get_pool(base).lookup_str(id.id, SOLVABLE_DESCRIPTION));
}

AdvisoryId Advisory::get_id() const {
    return id;
}

std::vector<AdvisoryReference> Advisory::get_references(std::vector<std::string> types) const {
    auto & pool = get_pool(base);

    std::vector<AdvisoryReference> output;

    Dataiterator di;
    dataiterator_init(&di, *pool, 0, id.id, UPDATE_REFERENCE, 0, 0);

    for (int index = 0; dataiterator_step(&di); index++) {
        dataiterator_setpos(&di);
        std::string current_type = std::string(pool.lookup_str(SOLVID_POS, UPDATE_REFERENCE_TYPE));

        if(types.empty() || std::find(types.begin(), types.end(), current_type) != types.end()) {
            output.emplace_back(AdvisoryReference(base, id, index));
        }
    }

    dataiterator_free(&di);
    return output;
}

std::vector<AdvisoryCollection> Advisory::get_collections() const {
    std::vector<AdvisoryCollection> output;

    Dataiterator di;
    dataiterator_init(&di, *get_pool(base), 0, id.id, UPDATE_COLLECTIONLIST, 0, 0);

    for (int index = 0; dataiterator_step(&di); index++) {
        dataiterator_setpos(&di);
        output.emplace_back(AdvisoryCollection(base, id, index));
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
