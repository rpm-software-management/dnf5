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


#ifndef TEST_LIBDNF5_UTILS_HPP
#define TEST_LIBDNF5_UTILS_HPP

#include "system/state.hpp"
#include "utils/string.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/advisory/advisory_set.hpp>
#include <libdnf5/base/transaction.hpp>
#include <libdnf5/base/transaction_package.hpp>
#include <libdnf5/comps/environment/environment.hpp>
#include <libdnf5/comps/environment/query.hpp>
#include <libdnf5/comps/group/group.hpp>
#include <libdnf5/comps/group/query.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/package_set.hpp>
#include <transaction/transaction_sr.hpp>

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
struct assertion_traits<libdnf5::advisory::Advisory> {
    inline static bool equal(const libdnf5::advisory::Advisory & left, const libdnf5::advisory::Advisory & right) {
        return left == right;
    }

    inline static std::string toString(const libdnf5::advisory::Advisory & advisory) {
        return fmt::format("{} (id: {})", advisory.get_name(), advisory.get_id().id);
    }
};

template <>
struct assertion_traits<libdnf5::advisory::AdvisorySet> {
    inline static std::string toString(const libdnf5::advisory::AdvisorySet & advisories) {
        std::string result;

        for (const auto & advisory : advisories) {
            result += "\n    " + assertion_traits<libdnf5::advisory::Advisory>::toString(advisory);
        }

        return result;
    }
};

template <>
struct assertion_traits<libdnf5::comps::Environment> {
    inline static bool equal(const libdnf5::comps::Environment & left, const libdnf5::comps::Environment & right) {
        return left == right;
    }

    inline static std::string toString(const libdnf5::comps::Environment & environment) {
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
struct assertion_traits<libdnf5::Set<libdnf5::comps::Environment>> {
    inline static std::string toString(const libdnf5::Set<libdnf5::comps::Environment> & environments) {
        std::string result;

        for (const auto & environment : environments) {
            result += "\n    " + assertion_traits<libdnf5::comps::Environment>::toString(environment);
        }

        return result;
    }
};

template <>
struct assertion_traits<libdnf5::comps::Group> {
    inline static bool equal(const libdnf5::comps::Group & left, const libdnf5::comps::Group & right) {
        return left == right;
    }

    inline static std::string toString(const libdnf5::comps::Group & group) {
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
struct assertion_traits<libdnf5::Set<libdnf5::comps::Group>> {
    inline static std::string toString(const libdnf5::Set<libdnf5::comps::Group> & groups) {
        std::string result;

        for (const auto & group : groups) {
            result += "\n    " + assertion_traits<libdnf5::comps::Group>::toString(group);
        }

        return result;
    }
};

template <>
struct assertion_traits<libdnf5::rpm::Package> {
    inline static bool equal(const libdnf5::rpm::Package & left, const libdnf5::rpm::Package & right) {
        return left == right;
    }

    inline static std::string toString(const libdnf5::rpm::Package & pkg) {
        return fmt::format(
            "{} (id: {} repo: {} {})",
            pkg.get_full_nevra(),
            pkg.get_id().id,
            pkg.get_repo_id(),
            pkg.is_installed() ? "installed" : "available");
    }
};

template <>
struct assertion_traits<libdnf5::rpm::PackageQuery> {
    inline static std::string toString(const libdnf5::rpm::PackageQuery & pkg_query) {
        std::string result;

        for (const auto & pkg : pkg_query) {
            result += "\n    " + assertion_traits<libdnf5::rpm::Package>::toString(pkg);
        }

        return result;
    }
};

template <>
struct assertion_traits<libdnf5::rpm::Reldep> {
    inline static bool equal(const libdnf5::rpm::Reldep & left, const libdnf5::rpm::Reldep & right) {
        return left == right;
    }

    inline static std::string toString(const libdnf5::rpm::Reldep & reldep) {
        return fmt::format("{} (id: {})", reldep.to_string(), reldep.get_id().id);
    }
};

template <>
struct assertion_traits<libdnf5::base::TransactionPackage> {
    inline static bool equal(
        const libdnf5::base::TransactionPackage & left, const libdnf5::base::TransactionPackage & right) {
        return left.get_package() == right.get_package() && left.get_action() == right.get_action() &&
               left.get_reason() == right.get_reason() && left.get_state() == right.get_state();
    }

    inline static std::string toString(const libdnf5::base::TransactionPackage & tspkg) {
        return fmt::format(
            "TransactionPackage: package: {}, action: {}, reason: {}, state {}",
            to_string(tspkg.get_package()),
            transaction_item_action_to_string(tspkg.get_action()),
            transaction_item_reason_to_string(tspkg.get_reason()),
            transaction_item_state_to_string(tspkg.get_state()));
    }
};

template <>
struct assertion_traits<libdnf5::system::PackageState> {
    inline static bool equal(const libdnf5::system::PackageState & left, const libdnf5::system::PackageState & right) {
        return left.reason == right.reason;
    }

    inline static std::string toString(const libdnf5::system::PackageState & pkg_state) {
        return fmt::format("PackageState: reason: {}", pkg_state.reason);
    }
};

template <>
struct assertion_traits<libdnf5::system::NevraState> {
    inline static bool equal(const libdnf5::system::NevraState & left, const libdnf5::system::NevraState & right) {
        return left.from_repo == right.from_repo;
    }

    inline static std::string toString(const libdnf5::system::NevraState & nevra_state) {
        return fmt::format("NevraState: from_repo: {}", nevra_state.from_repo);
    }
};

template <>
struct assertion_traits<libdnf5::system::GroupState> {
    inline static bool equal(const libdnf5::system::GroupState & left, const libdnf5::system::GroupState & right) {
        return left.userinstalled == right.userinstalled && left.packages == right.packages &&
               static_cast<int>(left.package_types) == static_cast<int>(right.package_types);
    }

    inline static std::string toString(const libdnf5::system::GroupState & group_state) {
        return fmt::format(
            "GroupState: userinstalled: {}, packages: {}, package_types: {}",
            group_state.userinstalled,
            assertion_traits<std::vector<std::string>>::toString(group_state.packages),
            assertion_traits<std::vector<std::string>>::toString(
                libdnf5::comps::package_types_to_strings(group_state.package_types)));
    }
};

#ifdef WITH_MODULEMD
template <>
struct assertion_traits<libdnf5::system::ModuleState> {
    inline static bool equal(const libdnf5::system::ModuleState & left, const libdnf5::system::ModuleState & right) {
        return left.enabled_stream == right.enabled_stream && left.status == right.status &&
               left.installed_profiles == right.installed_profiles;
    }

    inline static std::string toString(const libdnf5::system::ModuleState & module_state) {
        return fmt::format(
            "ModuleState: enabled_stream: {}, state: {}, installed_profiles: {}",
            module_state.enabled_stream,
            libdnf5::module::module_status_to_string(module_state.status),
            assertion_traits<std::vector<std::string>>::toString(module_state.installed_profiles));
    }
};
#endif

template <>
struct assertion_traits<libdnf5::transaction::PackageReplay> {
    inline static bool equal(
        const libdnf5::transaction::PackageReplay & left, const libdnf5::transaction::PackageReplay & right) {
        return left.action == right.action && left.reason == right.reason && left.group_id == right.group_id &&
               left.nevra == right.nevra && left.package_path == right.package_path && left.repo_id == right.repo_id;
    }

    inline static std::string toString(const libdnf5::transaction::PackageReplay & replay) {
        return fmt::format(
            "PackageReplay: nevra: {}, action: {}, reason: {}, repo_id: {}, group_id: {}, package_path: {}",
            replay.nevra,
            libdnf5::transaction::transaction_item_action_to_string(replay.action),
            libdnf5::transaction::transaction_item_reason_to_string(replay.reason),
            replay.repo_id,
            replay.group_id,
            std::string(replay.package_path));
    }
};

template <>
struct assertion_traits<libdnf5::transaction::GroupReplay> {
    inline static bool equal(
        const libdnf5::transaction::GroupReplay & left, const libdnf5::transaction::GroupReplay & right) {
        return left.action == right.action && left.reason == right.reason && left.group_id == right.group_id &&
               left.group_path == right.group_path && left.repo_id == right.repo_id &&
               left.package_types == right.package_types;
    }

    inline static std::string toString(const libdnf5::transaction::GroupReplay & replay) {
        return fmt::format(
            "GroupReplay: group_id: {}, action: {}, reason: {}, repo_id: {}, group_path: {}, package_types: {}",
            replay.group_id,
            libdnf5::transaction::transaction_item_action_to_string(replay.action),
            libdnf5::transaction::transaction_item_reason_to_string(replay.reason),
            replay.repo_id,
            std::string(replay.group_path),
            libdnf5::utils::string::join(package_types_to_strings(replay.package_types), ","));
    }
};

template <>
struct assertion_traits<libdnf5::transaction::EnvironmentReplay> {
    inline static bool equal(
        const libdnf5::transaction::EnvironmentReplay & left, const libdnf5::transaction::EnvironmentReplay & right) {
        return left.action == right.action && left.environment_id == right.environment_id &&
               left.environment_path == right.environment_path && left.repo_id == right.repo_id;
    }

    inline static std::string toString(const libdnf5::transaction::EnvironmentReplay & replay) {
        return fmt::format(
            "EnvironmentReplay: environment_id: {}, action: {}, repo_id: {}, environment_path: {}",
            replay.environment_id,
            libdnf5::transaction::transaction_item_action_to_string(replay.action),
            replay.repo_id,
            std::string(replay.environment_path));
    }
};

}  // namespace CPPUNIT_NS


std::vector<libdnf5::advisory::Advisory> to_vector(const libdnf5::advisory::AdvisorySet & advisory_set);
std::vector<libdnf5::comps::Environment> to_vector(const libdnf5::Set<libdnf5::comps::Environment> & environment_set);
std::vector<libdnf5::comps::Group> to_vector(const libdnf5::Set<libdnf5::comps::Group> & group_set);
std::vector<libdnf5::rpm::Reldep> to_vector(const libdnf5::rpm::ReldepList & reldep_list);
std::vector<libdnf5::rpm::Package> to_vector(const libdnf5::rpm::PackageSet & package_set);

#endif  // TEST_LIBDNF5_UTILS_HPP
