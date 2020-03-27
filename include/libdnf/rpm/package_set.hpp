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


#ifndef LIBDNF_RPM_PACKAGE_SET_HPP
#define LIBDNF_RPM_PACKAGE_SET_HPP


#include "package.hpp"
#include "sack.hpp"

#include <solv/bitmap.h>
#include <solv/pooltypes.h>

#include <memory>


namespace libdnf::rpm {


class PackageSet {
public:
    explicit PackageSet(Sack * sack);
    PackageSet(Sack * sack, Map * map);
    PackageSet(const PackageSet & pset);
    PackageSet(PackageSet && pset) noexcept;
    ~PackageSet();
    Id operator[](unsigned int index) const;
    PackageSet & operator+=(const PackageSet & other);
    PackageSet & operator-=(const PackageSet & other);
    PackageSet & operator/=(const PackageSet & other);
    PackageSet & operator+=(const Map * other);
    PackageSet & operator-=(const Map * other);
    PackageSet & operator/=(const Map * other);
    void clear();
    bool empty();
    void set(const Package & pkg);
    void set(Id id);
    bool has(const Package & pkg) const;
    bool has(Id id) const;
    void remove(Id id);
    Map * get_map() const;
    Sack * get_sack() const;
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


}  // namespace libdnf::rpm


#endif // LIBDNF_RPM_PACKAGE_SET_HPP
