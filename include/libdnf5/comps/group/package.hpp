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

#include "libdnf5/common/exception.hpp"

#include <string>
#include <vector>


namespace libdnf5::comps {

enum class PackageType : int {
    CONDITIONAL = 1 << 0,  // a weak dependency
    DEFAULT = 1 << 1,      // installed by default, but can be unchecked in the UI
    MANDATORY = 1 << 2,    // installed
    OPTIONAL = 1 << 3      // not installed by default, but can be checked in the UI
};

class InvalidPackageType : public libdnf5::Error {
public:
    InvalidPackageType(const std::string & type);
    InvalidPackageType(const PackageType type);

    const char * get_domain_name() const noexcept override { return "libdnf::comps"; }
    const char * get_name() const noexcept override { return "InvalidPackageType"; }
};

inline PackageType operator|(PackageType a, PackageType b) {
    return static_cast<PackageType>(
        static_cast<std::underlying_type<PackageType>::type>(a) |
        static_cast<std::underlying_type<PackageType>::type>(b));
}

inline PackageType operator|=(PackageType & a, PackageType b) {
    a = static_cast<PackageType>(
        static_cast<std::underlying_type<PackageType>::type>(a) |
        static_cast<std::underlying_type<PackageType>::type>(b));
    return a;
}

inline constexpr PackageType operator&(PackageType a, PackageType b) {
    return static_cast<PackageType>(
        static_cast<std::underlying_type<PackageType>::type>(a) &
        static_cast<std::underlying_type<PackageType>::type>(b));
}

inline constexpr bool any(PackageType flags) {
    return static_cast<std::underlying_type<PackageType>::type>(flags) != 0;
}

PackageType package_type_from_string(const std::string & type);
PackageType package_type_from_string(const std::vector<std::string> types);
std::string package_type_to_string(const PackageType type);
std::vector<std::string> package_types_to_strings(const PackageType types);


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


}  // namespace libdnf5::comps


#endif
