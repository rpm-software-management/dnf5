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

#include <stdexcept>
#include "Dependency.hpp"
#include "libdnf/utils/utils.hpp"
#include "libdnf/repo/DependencySplitter.hpp"

/* workaround, libsolv lacks 'extern "C"' in its header file */
extern "C" {
#include <solv/pool_parserpmrichdep.h>
#include <solv/util.h>
}

#include <stdexcept>

namespace libdnf {

static int transformToLibsolvComparisonType(int cmp_type)
{
    int type = 0;
    if (cmp_type & HY_EQ)
        type |= REL_EQ;
    if (cmp_type & HY_LT)
        type |= REL_LT;
    if (cmp_type & HY_GT)
        type |= REL_GT;

    return type;
}

Dependency::Dependency(DnfSack *sack, Id id)
        : sack(sack)
        , id(id)
{}

Dependency::Dependency(DnfSack *sack, const char *name, const char *version, int cmpType)
        : sack(sack)
{
    id = getReldepId(sack, name, version, cmpType);
}

Dependency::Dependency(DnfSack *sack, const std::string &dependency)
        : sack(sack)
{
    id = getReldepId(sack, dependency.c_str());
}


Dependency::Dependency(const Dependency &dependency)
        : sack(dependency.sack)
        , id(dependency.id)
{}

Dependency::~Dependency() = default;
const char *Dependency::getName() const { return pool_id2str(dnf_sack_get_pool(sack), id); }
const char *Dependency::getRelation() const { return pool_id2rel(dnf_sack_get_pool(sack), id); }
const char *Dependency::getVersion() const { return pool_id2evr(dnf_sack_get_pool(sack), id); }
const char *Dependency::toString() const { return pool_dep2str(dnf_sack_get_pool(sack), id); }

Id
Dependency::getReldepId(DnfSack *sack, const char *name, const char *version, int cmpType)
{
    Id id;
    int solvComparisonOperator = transformToLibsolvComparisonType(cmpType);
    Pool *pool = dnf_sack_get_pool(sack);
    id = pool_str2id(pool, name, 1);

    if (version) {
        Id evrId = pool_str2id(pool, version, 1);
        id = pool_rel2id(pool, id, evrId, solvComparisonOperator, 1);
    }
    return id;
}

Id
Dependency::getReldepId(DnfSack *sack, const char * reldepStr)
{
    if (reldepStr[0] == '(') {
        /* Rich dependency */
        Pool *pool = dnf_sack_get_pool (sack);
        Id id = pool_parserpmrichdep(pool, reldepStr);
        if (!id)
            throw std::runtime_error("Cannot parse a dependency string");
        return id;
    } else {
        DependencySplitter depSplitter;
        if(!depSplitter.parse(reldepStr))
            throw std::runtime_error("Cannot parse a dependency string");
        return getReldepId(sack, depSplitter.getNameCStr(), depSplitter.getEVRCStr(),
                           depSplitter.getCmpType());
    }
}

}
