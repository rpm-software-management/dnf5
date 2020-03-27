/*
 * Copyright (C) 2018 Red Hat, Inc.
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <assert.h>

#include "libdnf/rpm/package_set.hpp"
#include "../dnf-sack.h"
#include "../hy-util-private.hpp"

namespace libdnf {

class PackageSet::Impl {
public:
    Impl(DnfSack* sack);
    Impl(DnfSack* sack, Map* map);
    Impl(const PackageSet & pset);
    ~Impl();

private:
    friend PackageSet;
    DnfSack *sack;
    Map map;
};

PackageSet::PackageSet(DnfSack* sack) : pImpl(new Impl(sack)) {}
PackageSet::PackageSet(DnfSack* sack, Map* map_source) : pImpl(new Impl(sack, map_source)) {}
PackageSet::PackageSet(const PackageSet & pset): pImpl(new Impl(pset)) {}
PackageSet::PackageSet(PackageSet && pset): pImpl(std::move(pset.pImpl)) {}
PackageSet::~PackageSet() = default;

PackageSet::Impl::Impl(DnfSack* sack) :
sack(sack)
{
    map_init(&map, dnf_sack_get_pool(sack)->nsolvables);
}
PackageSet::Impl::Impl(DnfSack* sack, Map* map_source) : sack(sack)
{
    map_init_clone(&map, map_source);
}
PackageSet::Impl::Impl(const PackageSet & pset): sack(pset.pImpl->sack)
{
    map_init_clone(&map, &pset.pImpl->map);
}
PackageSet::Impl::~Impl() { map_free(&map); }

Id
PackageSet::operator [](unsigned int index) const
{
    const unsigned char *ti = pImpl->map.map;
    const unsigned char *end = ti + pImpl->map.size;
    unsigned int enabled;
    Id id;

    while (ti < end) {
        enabled = _BitCountLookup[*ti];

        if (index >= enabled ){
            index -= enabled;
            ti++;
            continue;
        }
        id = (ti - pImpl->map.map) << 3;

        index++;
        for (unsigned char byte = *ti; index; byte >>= 1) {
            if ((byte & 0x01))
                index--;
            if (index)
                id++;
        }
        return id;
    }
    return -1;
}

PackageSet &
PackageSet::operator +=(const PackageSet & other)
{
    map_or(&pImpl->map, &other.pImpl->map);
    return *this;
}

PackageSet &
PackageSet::operator -=(const PackageSet & other)
{
    map_subtract(&pImpl->map, &other.pImpl->map);
    return *this;
}

PackageSet &
PackageSet::operator /=(const PackageSet & other)
{
    map_and(&pImpl->map, &other.pImpl->map);
    return *this;
}

PackageSet &
PackageSet::operator +=(const Map * other)
{
    map_or(&pImpl->map, const_cast<Map *>(other));
    return *this;
}

PackageSet &
PackageSet::operator -=(const Map * other)
{
    map_subtract(&pImpl->map, const_cast<Map *>(other));
    return *this;
}

PackageSet &
PackageSet::operator /=(const Map * other)
{
    map_and(&pImpl->map, const_cast<Map *>(other));
    return *this;
}

void
PackageSet::clear()
{
    map_empty(&pImpl->map);
}

bool
PackageSet::empty()
{
    const unsigned char *res = pImpl->map.map;
    const unsigned char *end = res + pImpl->map.size;

    while (res < end) {
        if (*res++)
            return false;
    }
    return true;
}


void PackageSet::set(DnfPackage *pkg) { MAPSET(&pImpl->map, dnf_package_get_id(pkg)); }
void PackageSet::set(Id id) { MAPSET(&pImpl->map, id); }
bool PackageSet::has(DnfPackage *pkg) const { return MAPTST(&pImpl->map, dnf_package_get_id(pkg)); }
bool PackageSet::has(Id id) const { return MAPTST(&pImpl->map, id); }
void PackageSet::remove(Id id) { MAPCLR(&pImpl->map, id); }
Map *PackageSet::getMap() const { return &pImpl->map; }
DnfSack *PackageSet::getSack() const { return pImpl->sack; }
size_t PackageSet::size() const { return map_count(&pImpl->map); }

Id PackageSet::next(Id previous) const
{
    const unsigned char *ti = pImpl->map.map;
    const unsigned char *end = ti + pImpl->map.size;
    Id id;

    if (previous >= 0) {
        ti += previous >> 3;
        unsigned char byte = *ti; // byte with the previous match
        byte >>= (previous & 7) + 1; // shift away all previous 1 bits

        for (id = previous + 1; byte; byte >>= 1, id++)
            if (byte & 0x01)
                return id;
        ti++;
    }

    while (ti < end) {

        if (!*ti){
            ti++;
            continue;
        }
        id = (ti - pImpl->map.map) << 3;
        for (unsigned char byte = *ti; 1; byte >>= 1, id++) {
            if (byte & 0x01)
                return id;
        }
    }
    return -1;
}

}
