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

#include "libdnf/advisory/advisory_set.hpp"
#include "libdnf/base/transaction.hpp"
#include "libdnf/comps/group/group.hpp"
#include "libdnf/comps/group/query.hpp"
#include "libdnf/rpm/package_query.hpp"
#include "libdnf/rpm/package_set.hpp"
#include "libdnf/utils/format.hpp"

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
        return libdnf::utils::sformat("{} (id: {})", advisory.get_name(), advisory.get_id().id);
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

        return libdnf::utils::sformat("{} (repos: {})", group.get_groupid(), repos);
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
        return libdnf::utils::sformat(
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
        return libdnf::utils::sformat("{} (id: {})", reldep.to_string(), reldep.get_id().id);
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
        return libdnf::utils::sformat(
            "TransactionPackage: package: {}, action: {}, reason: {}, state {}",
            to_string(tspkg.get_package()),
            transaction_item_action_to_string(tspkg.get_action()),
            transaction_item_reason_to_string(tspkg.get_reason()),
            transaction_item_state_to_string(tspkg.get_state()));
    }
};

}  // namespace CPPUNIT_NS


std::vector<libdnf::advisory::Advisory> to_vector(const libdnf::advisory::AdvisorySet & advisory_set);
std::vector<libdnf::comps::Group> to_vector(const libdnf::Set<libdnf::comps::Group> & group_set);
std::vector<libdnf::rpm::Reldep> to_vector(const libdnf::rpm::ReldepList & reldep_list);
std::vector<libdnf::rpm::Package> to_vector(const libdnf::rpm::PackageSet & package_set);

#endif  // TEST_LIBDNF_UTILS_HPP
