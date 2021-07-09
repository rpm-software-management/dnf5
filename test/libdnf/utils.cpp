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


std::vector<libdnf::rpm::Reldep> to_vector(const libdnf::rpm::ReldepList & reldep_list) {
    std::vector<libdnf::rpm::Reldep> res;
    for (const auto & reldep : reldep_list) {
        res.push_back(reldep);
    }
    return res;
}


std::vector<libdnf::rpm::Package> to_vector(const libdnf::rpm::PackageSet & package_set) {
    std::vector<libdnf::rpm::Package> res;
    for (const auto & pkg : package_set) {
        res.push_back(pkg);
    }
    return res;
}


std::vector<libdnf::advisory::Advisory> to_vector(const libdnf::advisory::AdvisorySet & advisory_set) {
    std::vector<libdnf::advisory::Advisory> res;
    for (const auto & advisory : advisory_set) {
        res.push_back(advisory);
    }
    return res;
}


std::vector<libdnf::comps::Environment> to_vector(const libdnf::Set<libdnf::comps::Environment> & environment_set) {
    std::vector<libdnf::comps::Environment> res;
    for (const auto & environment : environment_set) {
        res.push_back(environment);
    }
    return res;
}


std::vector<libdnf::comps::Group> to_vector(const libdnf::Set<libdnf::comps::Group> & group_set) {
    std::vector<libdnf::comps::Group> res;
    for (const auto & group : group_set) {
        res.push_back(group);
    }
    return res;
}
