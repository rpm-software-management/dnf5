// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "libdnf5/comps/group/package.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <string>
#include <vector>


namespace libdnf5::comps {

class Package::Impl {
public:
    Impl(std::string name, PackageType type, std::string condition)
        : name(std::move(name)),
          type(type),
          condition(std::move(condition)) {}

private:
    friend Package;

    std::string name;
    PackageType type;
    // Used only for CONDITIONAL packages
    std::string condition;
};

Package::Package(const std::string & name, PackageType type, const std::string & condition)
    : p_impl(std::make_unique<Impl>(name, type, condition)) {}

Package::~Package() = default;

Package::Package(const Package & src) : p_impl(new Impl(*src.p_impl)) {}
Package::Package(Package && src) noexcept = default;

Package & Package::operator=(const Package & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
Package & Package::operator=(Package && src) noexcept = default;

bool Package::operator==(const Package & other) const noexcept {
    return p_impl->name == other.p_impl->name && p_impl->type == other.p_impl->type &&
           p_impl->condition == other.p_impl->condition;
}

bool Package::operator!=(const Package & other) const noexcept {
    return !(*this == other);
}

std::string Package::get_name() const {
    return p_impl->name;
}
void Package::set_name(const std::string & value) {
    p_impl->name = value;
}

/// @return The PackageType.
/// @since 5.0
PackageType Package::get_type() const {
    return p_impl->type;
}
void Package::set_type(const PackageType & value) {
    p_impl->type = value;
}

/// @return std::string that corresponds to the PackageType.
/// @since 5.0
std::string Package::get_type_string() const {
    switch (p_impl->type) {
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
std::string Package::get_condition() const {
    return p_impl->condition;
}
void Package::set_condition(const std::string & value) {
    p_impl->condition = value;
}

InvalidPackageType::InvalidPackageType(const std::string & type)
    : libdnf5::Error(M_("Invalid package type: {}"), type) {}

InvalidPackageType::InvalidPackageType(const PackageType type)
    : libdnf5::Error(M_("Invalid package type: {}"), static_cast<int>(type)) {}

PackageType package_type_from_string(const std::string & type) {
    if (type == "mandatory") {
        return PackageType::MANDATORY;
    } else if (type == "default") {
        return PackageType::DEFAULT;
    } else if (type == "conditional") {
        return PackageType::CONDITIONAL;
    } else if (type == "optional") {
        return PackageType::OPTIONAL;
    }
    throw InvalidPackageType(type);
}

PackageType package_type_from_string(const std::vector<std::string> types) {
    PackageType retval = static_cast<PackageType>(0);
    for (const auto & type : types) {
        retval |= package_type_from_string(type);
    }
    return retval;
}

std::string package_type_to_string(const PackageType type) {
    switch (type) {
        case PackageType::MANDATORY:
            return "mandatory";
        case PackageType::DEFAULT:
            return "default";
        case PackageType::CONDITIONAL:
            return "conditional";
        case PackageType::OPTIONAL:
            return "optional";
    }
    throw InvalidPackageType(type);
}

std::vector<std::string> package_types_to_strings(const PackageType types) {
    std::vector<std::string> result_types;
    for (const auto available_type : std::vector<PackageType>{
             PackageType::MANDATORY, PackageType::DEFAULT, PackageType::CONDITIONAL, PackageType::OPTIONAL}) {
        if (any(types & available_type)) {
            result_types.push_back(package_type_to_string(available_type));
        }
    }
    return result_types;
}

}  // namespace libdnf5::comps
