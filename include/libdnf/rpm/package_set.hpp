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

#ifndef __PACKAGE_SET_HPP
#define __PACKAGE_SET_HPP

#include <memory>
#include <solv/bitmap.h>
#include "../dnf-types.h"
#include <solv/pooltypes.h>
#include "../hy-package.h"

namespace libdnf {

struct PackageSet {
public:
    PackageSet(DnfSack* sack);
    PackageSet(DnfSack* sack, Map* map);
    PackageSet(const PackageSet & pset);
    PackageSet(PackageSet && pset);
    ~PackageSet();
    Id operator [](unsigned int index) const;
    PackageSet & operator +=(const PackageSet & other);
    PackageSet & operator -=(const PackageSet & other);
    PackageSet & operator /=(const PackageSet & other);
    PackageSet & operator +=(const Map * other);
    PackageSet & operator -=(const Map * other);
    PackageSet & operator /=(const Map * other);
    void clear();
    bool empty();
    void set(DnfPackage *pkg);
    void set(Id id);
    bool has(DnfPackage *pkg) const;
    bool has(Id id) const;
    void remove(Id id);
    Map *getMap() const;
    DnfSack *getSack() const;
    size_t size() const;

    /**
    * @brief Returns next id in packageset or -1 if end of package set reached
    *
    * @param previous Id of previous element
    * @return Id
    */
    Id next(Id previous) const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

}

#endif /* __PACKAGE_SET_HPP */
