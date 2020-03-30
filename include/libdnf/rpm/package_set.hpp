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


/// @replaces libdnf:sack/packageset.hpp:struct:PackageSet
class PackageSet {
public:
    /// @replaces libdnf:hy-packageset.h:function:dnf_packageset_new(DnfSack * sack)
    explicit PackageSet(Sack * sack);

    /// @replaces libdnf:hy-packageset.h:function:dnf_packageset_from_bitmap(DnfSack * sack, Map * m)
    PackageSet(Sack * sack, Map * map);

    /// @replaces libdnf:hy-packageset.h:function:dnf_packageset_clone(DnfPackageSet * pset)
    PackageSet(const PackageSet & pset);

    PackageSet(PackageSet && pset) noexcept;

    /// @replaces libdnf:hy-packageset.h:function:dnf_packageset_free(DnfPackageSet * pset)
    ~PackageSet();

    /// @replaces libdnf:sack/packageset.hpp:method:PackageSet.operator[](unsigned int index)
    Id operator[](unsigned int index) const;

    /// @replaces libdnf:sack/packageset.hpp:method:PackageSet.operator+=(const libdnf::PackageSet & other)
    PackageSet & operator|=(const PackageSet & other);

    /// @replaces libdnf:sack/packageset.hpp:method:PackageSet.operator-=(const libdnf::PackageSet & other)
    PackageSet & operator-=(const PackageSet & other);

    /// @replaces libdnf:sack/packageset.hpp:method:PackageSet.operator/=(const libdnf::PackageSet & other)
    PackageSet & operator&=(const PackageSet & other);

    /// @replaces libdnf:sack/packageset.hpp:method:PackageSet.operator+=(const Map * other)
    PackageSet & operator|=(const Map * other);

    /// @replaces libdnf:sack/packageset.hpp:method:PackageSet.operator-=(const Map * other)
    PackageSet & operator-=(const Map * other);

    /// @replaces libdnf:sack/packageset.hpp:method:PackageSet.operator/=(const Map * other)
    PackageSet & operator&=(const Map * other);

    /// @replaces libdnf:sack/packageset.hpp:method:PackageSet.clear()
    void clear();

    /// @replaces libdnf:sack/packageset.hpp:method:PackageSet.empty()
    bool empty();

    /// @replaces libdnf:sack/packageset.hpp:method:PackageSet.set(DnfPackage * pkg)
    /// @replaces libdnf:hy-packageset.h:function:dnf_packageset_add(DnfPackageSet * pset, DnfPackage * pkg)
    void set(const Package & pkg);

    /// @replaces libdnf:sack/packageset.hpp:method:PackageSet.set(Id id)
    void set(Id id);

    /// @replaces libdnf:sack/packageset.hpp:method:PackageSet.has(DnfPackage * pkg)
    bool has(const Package & pkg) const;

    /// @replaces libdnf:sack/packageset.hpp:method:PackageSet.has(Id id)
    /// @replaces libdnf:hy-packageset.h:function:dnf_packageset_has(DnfPackageSet * pset, DnfPackage * pkg)
    bool has(Id id) const;

    /// @replaces libdnf:sack/packageset.hpp:method:PackageSet.remove(Id id)
    void remove(Id id);

    /// @replaces libdnf:sack/packageset.hpp:method:PackageSet.getMap()
    /// @replaces libdnf:hy-packageset.h:function:dnf_packageset_get_map(DnfPackageSet * pset)
    Map * get_map() const;

    /// @replaces libdnf:sack/packageset.hpp:method:PackageSet.getSack()
    Sack * get_sack() const;

    /// @replaces libdnf:sack/packageset.hpp:method:PackageSet.size()
    /// @replaces libdnf:hy-packageset.h:function:dnf_packageset_count(DnfPackageSet * pset)
    size_t size() const;

    /// @brief Returns next id in packageset or -1 if end of package set reached
    ///
    /// @param previous Id of previous element
    /// @return Id
    ///
    /// @replaces libdnf:sack/packageset.hpp:method:PackageSet.next(Id previous)
    Id next(Id previous) const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};


}  // namespace libdnf::rpm


#endif // LIBDNF_RPM_PACKAGE_SET_HPP
