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


#include "libdnf/rpm/package_set.hpp"


namespace libdnf::rpm {


class PackageSet::Impl {
public:
    Impl(Sack * sack);
    Impl(Sack * sack, Map * map);
    Impl(const PackageSet & pset);
    ~Impl();

private:
    friend PackageSet;
    Sack * sack;
    Map map;
};


PackageSet::PackageSet(Sack * sack) : pImpl(new Impl(sack)) {}
PackageSet::PackageSet(Sack * sack, Map * map_source) : pImpl(new Impl(sack, map_source)) {}
PackageSet::PackageSet(const PackageSet & pset) : pImpl(new Impl(pset)) {}
PackageSet::PackageSet(PackageSet && pset) noexcept : pImpl(std::move(pset.pImpl)) {}
PackageSet::~PackageSet() = default;


PackageSet::Impl::Impl(Sack * sack) : sack(sack) {
    map_init(&map, sack->get_pool()->nsolvables);
}


PackageSet::Impl::Impl(Sack * sack, Map * map_source) : sack(sack) {
    map_init_clone(&map, map_source);
}


PackageSet::Impl::Impl(const PackageSet & pset) : sack(pset.pImpl->sack) {
    map_init_clone(&map, &pset.pImpl->map);
}


PackageSet::Impl::~Impl() {
    map_free(&map);
}


// see http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetTable
static const unsigned char _BitCountLookup[256] = {
    #define B2(n) n, n + 1, n + 1, n + 2
    #define B4(n) B2(n), B2(n + 1), B2(n + 1), B2(n + 2)
    #define B6(n) B4(n), B4(n + 1), B4(n + 1), B4(n + 2)
    B6(0), B6(1), B6(1), B6(2)
};


inline size_t map_count(Map * m) {
    unsigned char * ti = m->map;
    unsigned char * end = ti + m->size;
    unsigned c = 0;

    while (ti < end) {
        c += _BitCountLookup[*ti++];
    }

    return c;
}


Id PackageSet::operator[](unsigned int index) const {
    const unsigned char * ti = pImpl->map.map;
    const unsigned char * end = ti + pImpl->map.size;
    unsigned int enabled;
    Id id;

    while (ti < end) {
        enabled = _BitCountLookup[*ti];

        if (index >= enabled) {
            index -= enabled;
            ti++;
            continue;
        }
        id = static_cast<Id>(ti - pImpl->map.map) << 3;

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


PackageSet & PackageSet::operator|=(const PackageSet & other) {
    map_or(&pImpl->map, &other.pImpl->map);
    return *this;
}


PackageSet & PackageSet::operator-=(const PackageSet & other) {
    map_subtract(&pImpl->map, &other.pImpl->map);
    return *this;
}


PackageSet & PackageSet::operator&=(const PackageSet & other) {
    map_and(&pImpl->map, &other.pImpl->map);
    return *this;
}


PackageSet & PackageSet::operator|=(const Map * other) {
    map_or(&pImpl->map, const_cast<Map *>(other));
    return *this;
}

PackageSet & PackageSet::operator-=(const Map * other) {
    map_subtract(&pImpl->map, const_cast<Map *>(other));
    return *this;
}


PackageSet & PackageSet::operator&=(const Map * other) {
    map_and(&pImpl->map, const_cast<Map *>(other));
    return *this;
}


void PackageSet::clear() {
    map_empty(&pImpl->map);
}


bool PackageSet::empty() {
    const unsigned char * res = pImpl->map.map;
    const unsigned char * end = res + pImpl->map.size;

    while (res < end) {
        if (*res++)
            return false;
    }
    return true;
}


void PackageSet::set(const Package & pkg) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    MAPSET(&pImpl->map, pkg.get_id());
#pragma GCC diagnostic pop
}


void PackageSet::set(Id id) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    MAPSET(&pImpl->map, id);
#pragma GCC diagnostic pop
}


bool PackageSet::has(const Package & pkg) const {
    return MAPTST(&pImpl->map, pkg.get_id());
}


bool PackageSet::has(Id id) const {
    return MAPTST(&pImpl->map, id);
}


void PackageSet::remove(Id id) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    MAPCLR(&pImpl->map, id);
#pragma GCC diagnostic pop
}


Map * PackageSet::get_map() const {
    return &pImpl->map;
}


Sack * PackageSet::get_sack() const {
    return pImpl->sack;
}


size_t PackageSet::size() const {
    return map_count(&pImpl->map);
}


Id PackageSet::next(Id previous) const {
    const unsigned char * ti = pImpl->map.map;
    const unsigned char * end = ti + pImpl->map.size;
    Id id;

    if (previous >= 0) {
        ti += previous >> 3;
        unsigned char byte = *ti;     // byte with the previous match
        byte >>= (previous & 7) + 1;  // shift away all previous 1 bits

        for (id = previous + 1; byte; byte >>= 1, id++)
            if (byte & 0x01)
                return id;
        ti++;
    }

    while (ti < end) {
        if (!*ti) {
            ti++;
            continue;
        }
        id = static_cast<Id>(ti - pImpl->map.map) << 3;
        for (unsigned char byte = *ti; 1; byte >>= 1, id++) {
            if (byte & 0x01)
                return id;
        }
    }
    return -1;
}


}  // namespace libdnf::rpm
