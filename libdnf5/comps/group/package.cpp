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

#include "libdnf5/comps/group/package.hpp"

#include "utils/bgettext/bgettext-mark-domain.h"

#include <string>
#include <vector>


namespace libdnf5::comps {

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
