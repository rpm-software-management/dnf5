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

#include "libdnf/advisory/advisory_reference.hpp"

#include "libdnf/logger/logger.hpp"
#include "libdnf/rpm/solv_sack_impl.hpp"

#include <solv/chksum.h>
#include <solv/repo.h>
#include <solv/util.h>

namespace libdnf::advisory {

inline const char * get_str_from_pool(Id keyname, Id advisory, Pool * pool, int index) {
    Dataiterator di;
    const char * str = NULL;
    int count = 0;

    dataiterator_init(&di, pool, 0, advisory, UPDATE_REFERENCE, 0, 0);
    while (dataiterator_step(&di)) {
        dataiterator_setpos(&di);
        if (count++ == index) {
            str = pool_lookup_str(pool, SOLVID_POS, keyname);
            break;
        }
    }
    dataiterator_free(&di);

    return str;
}

AdvisoryReference::AdvisoryReference(const libdnf::rpm::SolvSackWeakPtr & sack, AdvisoryId advisory, int index)
    : sack(sack)
    , advisory(advisory)
    , index(index) {}

std::string AdvisoryReference::get_id() const {
    Pool * pool = sack->p_impl->get_pool();
    return std::string(get_str_from_pool(UPDATE_REFERENCE_ID, advisory.id, pool, index));
}
AdvisoryReference::Type AdvisoryReference::get_type() const {
    Pool * pool = sack->p_impl->get_pool();
    const char * type = get_str_from_pool(UPDATE_REFERENCE_TYPE, advisory.id, pool, index);

    if (type == NULL)
        return Type::UNKNOWN;
    if (!g_strcmp0(type, "bugzilla"))
        return Type::BUGZILLA;
    if (!g_strcmp0(type, "cve"))
        return Type::CVE;
    if (!g_strcmp0(type, "vendor"))
        return Type::VENDOR;
    return Type::UNKNOWN;
}
std::string AdvisoryReference::get_title() const {
    Pool * pool = sack->p_impl->get_pool();
    return std::string(get_str_from_pool(UPDATE_REFERENCE_TITLE, advisory.id, pool, index));
}
std::string AdvisoryReference::get_url() const {
    Pool * pool = sack->p_impl->get_pool();
    return std::string(get_str_from_pool(UPDATE_REFERENCE_HREF, advisory.id, pool, index));
}

}  // namespace libdnf::advisory
