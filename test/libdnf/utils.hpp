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


#ifndef TEST_LIBDNF_UTILS_HPP
#define TEST_LIBDNF_UTILS_HPP

#include "system/state.hpp"

#include "libdnf/advisory/advisory_set.hpp"
#include "libdnf/base/transaction.hpp"
#include "libdnf/base/transaction_package.hpp"
#include "libdnf/comps/environment/environment.hpp"
#include "libdnf/comps/environment/query.hpp"
#include "libdnf/comps/group/group.hpp"
#include "libdnf/comps/group/query.hpp"
#include "libdnf/rpm/package_query.hpp"
#include "libdnf/rpm/package_set.hpp"

#include <cppunit/extensions/HelperMacros.h>

#include <iterator>
#include <vector>


template <typename T>
std::string to_string(const T & x) {
    return CPPUNIT_NS::assertion_traits<T>::toString(x);
}


namespace CPPUNIT_NS {

/// Implementation of assertions on std collections (e.g. asserting two vectors are equal).
/// https://stackoverflow.com/questions/43709391/cppunit-assert-equal-on-std-collections

template <template <typename...> class C, typename T>
struct assertion_traits<C<T>> {
    inline static bool equal(const C<T> & left, const C<T> & right) {
        if (left.size() != right.size()) {
            return false;
        }
        return std::equal(left.cbegin(), left.cend(), right.cbegin(), right.cend(), assertion_traits<T>::equal);
    }

    inline static std::string toString(const C<T> & items) {
        std::string result;

        for (const auto & item : items) {
            result += "\n    " + assertion_traits<T>::toString(item);
        }

        return result;
    }

    assertion_traits() = delete;
    ~assertion_traits() = delete;
    assertion_traits(const assertion_traits &) = delete;
    assertion_traits & operator=(const assertion_traits &) = delete;
};

template <>
struct assertion_traits<libdnf::advisory::Advisory> {
    inline static bool equal(const libdnf::advisory::Advisory & left, const libdnf::advisory::Advisory & right) {
        return left == right;
    }

    inline static std::string toString(const libdnf::advisory::Advisory & advisory) {
        return fmt::format("{} (id: {})", advisory.get_name(), advisory.get_id().id);
    }
};

template <>
struct assertion_traits<libdnf::advisory::AdvisorySet> {
    inline static std::string toString(const libdnf::advisory::AdvisorySet & advisories) {
        std::string result;

        for (const auto & advisory : advisories) {
            result += "\n    " + assertion_traits<libdnf::advisory::Advisory>::toString(advisory);
        }

        return result;
    }
};

template <>
struct assertion_traits<libdnf::comps::Environment> {
    inline static bool equal(const libdnf::comps::Environment & left, const libdnf::comps::Environment & right) {
        return left == right;
    }

    inline static std::string toString(const libdnf::comps::Environment & environment) {
        std::string repos;
        for (const auto & repo : environment.get_repos()) {
            if (!repos.empty()) {
                repos += ", ";
            }
            repos += repo;
        }

        return fmt::format("{} (repos: {})", environment.get_environmentid(), repos);
    }
};

template <>
struct assertion_traits<libdnf::Set<libdnf::comps::Environment>> {
    inline static std::string toString(const libdnf::Set<libdnf::comps::Environment> & environments) {
        std::string result;

        for (const auto & environment : environments) {
            result += "\n    " + assertion_traits<libdnf::comps::Environment>::toString(environment);
        }

        return result;
    }
};

template <>
struct assertion_traits<libdnf::comps::Group> {
    inline static bool equal(const libdnf::comps::Group & left, const libdnf::comps::Group & right) {
        return left == right;
    }

    inline static std::string toString(const libdnf::comps::Group & group) {
        std::string repos;
        for (const auto & repo : group.get_repos()) {
            if (!repos.empty()) {
                repos += ", ";
            }
            repos += repo;
        }

        return fmt::format("{} (repos: {})", group.get_groupid(), repos);
    }
};

template <>
struct assertion_traits<libdnf::Set<libdnf::comps::Group>> {
    inline static std::string toString(const libdnf::Set<libdnf::comps::Group> & groups) {
        std::string result;

        for (const auto & group : groups) {
            result += "\n    " + assertion_traits<libdnf::comps::Group>::toString(group);
        }

        return result;
    }
};

template <>
struct assertion_traits<libdnf::rpm::Package> {
    inline static bool equal(const libdnf::rpm::Package & left, const libdnf::rpm::Package & right) {
        return left == right;
    }

    inline static std::string toString(const libdnf::rpm::Package & pkg) {
        return fmt::format(
            "{} (id: {} repo: {} {})",
            pkg.get_full_nevra(),
            pkg.get_id().id,
            pkg.get_repo_id(),
            pkg.is_installed() ? "installed" : "available");
    }
};

template <>
struct assertion_traits<libdnf::rpm::PackageQuery> {
    inline static std::string toString(const libdnf::rpm::PackageQuery & pkg_query) {
        std::string result;

        for (const auto & pkg : pkg_query) {
            result += "\n    " + assertion_traits<libdnf::rpm::Package>::toString(pkg);
        }

        return result;
    }
};

template <>
struct assertion_traits<libdnf::rpm::Reldep> {
    inline static bool equal(const libdnf::rpm::Reldep & left, const libdnf::rpm::Reldep & right) {
        return left == right;
    }

    inline static std::string toString(const libdnf::rpm::Reldep & reldep) {
        return fmt::format("{} (id: {})", reldep.to_string(), reldep.get_id().id);
    }
};

template <>
struct assertion_traits<libdnf::base::TransactionPackage> {
    inline static bool equal(
        const libdnf::base::TransactionPackage & left, const libdnf::base::TransactionPackage & right) {
        return left.get_package() == right.get_package() && left.get_action() == right.get_action() &&
               left.get_reason() == right.get_reason() && left.get_state() == right.get_state();
    }

    inline static std::string toString(const libdnf::base::TransactionPackage & tspkg) {
        return fmt::format(
            "TransactionPackage: package: {}, action: {}, reason: {}, state {}",
            to_string(tspkg.get_package()),
            transaction_item_action_to_string(tspkg.get_action()),
            transaction_item_reason_to_string(tspkg.get_reason()),
            transaction_item_state_to_string(tspkg.get_state()));
    }
};

template <>
struct assertion_traits<libdnf::system::PackageState> {
    inline static bool equal(const libdnf::system::PackageState & left, const libdnf::system::PackageState & right) {
        return left.reason == right.reason;
    }

    inline static std::string toString(const libdnf::system::PackageState & pkg_state) {
        return fmt::format("PackageState: reason: {}", pkg_state.reason);
    }
};

template <>
struct assertion_traits<libdnf::system::NevraState> {
    inline static bool equal(const libdnf::system::NevraState & left, const libdnf::system::NevraState & right) {
        return left.from_repo == right.from_repo;
    }

    inline static std::string toString(const libdnf::system::NevraState & nevra_state) {
        return fmt::format("NevraState: from_repo: {}", nevra_state.from_repo);
    }
};

template <>
struct assertion_traits<libdnf::system::GroupPackage> {
    inline static bool equal(const libdnf::system::GroupPackage & left, const libdnf::system::GroupPackage & right) {
        return left.name == right.name && left.type == right.type && left.condition == right.condition;
    }

    inline static std::string toString(const libdnf::system::GroupPackage & group_package) {
        return fmt::format(
            "GroupPackage: name: {}, type: {}, condition: {}",
            group_package.name,
            libdnf::comps::package_type_to_string(group_package.type),
            group_package.condition);
    }
};

template <>
struct assertion_traits<libdnf::system::GroupState> {
    inline static bool equal(const libdnf::system::GroupState & left, const libdnf::system::GroupState & right) {
        return left.userinstalled == right.userinstalled && left.packages == right.packages &&
               left.name == right.name &&
               assertion_traits<std::vector<libdnf::system::GroupPackage>>::equal(
                   left.all_packages, right.all_packages);
    }

    inline static std::string toString(const libdnf::system::GroupState & group_state) {
        return fmt::format(
            "GroupState: userinstalled: {}, name: {}, packages: {}, all_packages: {}",
            group_state.userinstalled,
            group_state.name,
            assertion_traits<std::vector<std::string>>::toString(group_state.packages),
            assertion_traits<std::vector<libdnf::system::GroupPackage>>::toString(group_state.all_packages));
    }
};

template <>
struct assertion_traits<libdnf::system::ModuleState> {
    inline static bool equal(const libdnf::system::ModuleState & left, const libdnf::system::ModuleState & right) {
        return left.enabled_stream == right.enabled_stream && left.state == right.state &&
               left.installed_profiles == right.installed_profiles;
    }

    inline static std::string toString(const libdnf::system::ModuleState & module_state) {
        return fmt::format(
            "ModuleState: enabled_stream: {}, state: {}, installed_profiles: {}",
            module_state.enabled_stream,
            libdnf::module::module_state_to_string(module_state.state),
            assertion_traits<std::vector<std::string>>::toString(module_state.installed_profiles));
    }
};

}  // namespace CPPUNIT_NS


std::vector<libdnf::advisory::Advisory> to_vector(const libdnf::advisory::AdvisorySet & advisory_set);
std::vector<libdnf::comps::Environment> to_vector(const libdnf::Set<libdnf::comps::Environment> & environment_set);
std::vector<libdnf::comps::Group> to_vector(const libdnf::Set<libdnf::comps::Group> & group_set);
std::vector<libdnf::rpm::Reldep> to_vector(const libdnf::rpm::ReldepList & reldep_list);
std::vector<libdnf::rpm::Package> to_vector(const libdnf::rpm::PackageSet & package_set);

#endif  // TEST_LIBDNF_UTILS_HPP
