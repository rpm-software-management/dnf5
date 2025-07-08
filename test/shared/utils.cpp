/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "utils.hpp"


std::vector<libdnf5::rpm::Reldep> to_vector(const libdnf5::rpm::ReldepList & reldep_list) {
    std::vector<libdnf5::rpm::Reldep> res;
    for (const auto & reldep : reldep_list) {
        res.push_back(reldep);
    }
    return res;
}


std::vector<libdnf5::rpm::Package> to_vector(const libdnf5::rpm::PackageSet & package_set) {
    std::vector<libdnf5::rpm::Package> res;
    for (const auto & pkg : package_set) {
        res.push_back(pkg);
    }
    return res;
}


std::vector<libdnf5::advisory::Advisory> to_vector(const libdnf5::advisory::AdvisorySet & advisory_set) {
    std::vector<libdnf5::advisory::Advisory> res;
    for (const auto & advisory : advisory_set) {
        res.push_back(advisory);
    }
    return res;
}


std::vector<libdnf5::comps::EnvironmentWeakPtr> to_vector(
    const libdnf5::Set<libdnf5::comps::EnvironmentWeakPtr> & environment_set) {
    std::vector<libdnf5::comps::EnvironmentWeakPtr> res;
    for (const auto & environment : environment_set) {
        res.push_back(environment);
    }
    std::sort(res.begin(), res.end(), [](libdnf5::comps::EnvironmentWeakPtr a, libdnf5::comps::EnvironmentWeakPtr b) {
        return *a < *b;
    });
    return res;
}


std::vector<libdnf5::comps::GroupWeakPtr> to_vector(const libdnf5::Set<libdnf5::comps::GroupWeakPtr> & group_set) {
    std::vector<libdnf5::comps::GroupWeakPtr> res;
    for (const auto & group : group_set) {
        res.push_back(group);
    }
    std::sort(
        res.begin(), res.end(), [](libdnf5::comps::GroupWeakPtr a, libdnf5::comps::GroupWeakPtr b) { return *a < *b; });
    return res;
}
