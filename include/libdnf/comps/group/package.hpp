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

#ifndef LIBDNF_COMPS_GROUP_PACKAGE_HPP
#define LIBDNF_COMPS_GROUP_PACKAGE_HPP

#include <string>


namespace libdnf::comps {


enum class PackageType { MANDATORY, DEFAULT, OPTIONAL, CONDITIONAL };

// TODO(dmach): isn't it more a package dependency rather than a package?


// @replaces dnf:dnf/comps.py:class:Package
// @replaces dnf:dnf/comps.py:class:CompsTransPkg
class Package {
public:
    explicit Package(const std::string & name, PackageType type, const std::string & condition)
        : name(name),
          type(type),
          condition(condition) {}

    bool operator==(const Package & other) const noexcept {
        return name == other.name && type == other.type && condition == other.condition;
    }

    bool operator!=(const Package & other) const noexcept { return !(*this == other); }

    /// @return The Package name.
    /// @since 5.0
    //
    // @replaces dnf:dnf/comps.py:attribute:Package.name
    std::string get_name() const { return name; }
    void set_name(const std::string & value) { name = value; }

    /// @return The PackageType.
    /// @since 5.0
    PackageType get_type() const { return type; }
    void set_type(const PackageType & value) { type = value; }

    /// @return std::string that corresponds to the PackageType.
    /// @since 5.0
    std::string get_type_string() const {
        switch (type) {
            case PackageType::MANDATORY:
                return "mandatory";
            case PackageType::DEFAULT:
                return "default";
            case PackageType::OPTIONAL:
                return "optional";
            case PackageType::CONDITIONAL:
                return "conditional";
        }
        return "";
    }

    /// @return The condition (name of package) under which the package gets installed.
    /// @since 5.0
    std::string get_condition() const { return condition; }
    void set_condition(const std::string & value) { condition = value; }

private:
    std::string name;
    PackageType type;
    // Used only for CONDITIONAL packages
    std::string condition;
};


}  // namespace libdnf::comps


#endif
