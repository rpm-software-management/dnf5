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

#ifndef LIBDNF5_COMPS_GROUP_PACKAGE_HPP
#define LIBDNF5_COMPS_GROUP_PACKAGE_HPP

#include "package_type.hpp"

#include <string>

namespace libdnf5::comps {

// TODO(dmach): isn't it more a package dependency rather than a package?

// @replaces dnf:dnf/comps.py:class:Package
// @replaces dnf:dnf/comps.py:class:CompsTransPkg
class Package {
public:
    Package(const std::string & name, PackageType type, const std::string & condition);

    ~Package();

    Package(const Package & src);
    Package & operator=(const Package & src);

    Package(Package && src) noexcept;
    Package & operator=(Package && src) noexcept;

    bool operator==(const Package & other) const noexcept;

    bool operator!=(const Package & other) const noexcept;

    /// @return The Package name.
    /// @since 5.0
    //
    // @replaces dnf:dnf/comps.py:attribute:Package.name
    std::string get_name() const;
    void set_name(const std::string & value);

    /// @return The PackageType.
    /// @since 5.0
    PackageType get_type() const;
    void set_type(const PackageType & value);

    /// @return std::string that corresponds to the PackageType.
    /// @since 5.0
    std::string get_type_string() const;

    /// @return The condition (name of package) under which the package gets installed.
    /// @since 5.0
    std::string get_condition() const;
    void set_condition(const std::string & value);

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;
};


}  // namespace libdnf5::comps

#endif
