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

#ifndef LIBDNF5_MODULE_MODULE_QUERY_HPP
#define LIBDNF5_MODULE_MODULE_QUERY_HPP

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/common/sack/query.hpp"
#include "libdnf5/common/sack/query_cmp.hpp"
#include "libdnf5/common/weak_ptr.hpp"
#include "libdnf5/module/module_item.hpp"
#include "libdnf5/module/nsvcap.hpp"

#include <string>
#include <vector>


namespace libdnf5::module {

// TODO(pkratoch): Store pointers to ModuleItems instead of ModuleItems to allow faster copying
class ModuleQuery : public libdnf5::sack::Query<ModuleItem> {
public:
    /// Create a new ModuleQuery instance.
    ///
    /// @param base     A weak pointer to Base
    /// @param empty    `true` to create empty query, `false` to create query with all modules
    explicit ModuleQuery(const libdnf5::BaseWeakPtr & base, bool empty = false);

    ~ModuleQuery();

    ModuleQuery(const ModuleQuery & src);
    ModuleQuery & operator=(const ModuleQuery & src);

    ModuleQuery(ModuleQuery && src) noexcept;
    ModuleQuery & operator=(ModuleQuery && src) noexcept;

    /// Create a new ModuleQuery instance.
    ///
    /// @param base     Reference to Base
    /// @param empty    `true` to create empty query, `false` to create query with all modules
    explicit ModuleQuery(libdnf5::Base & base, bool empty = false);

    /// @return Weak pointer to the Base object.
    /// @since 5.0
    libdnf5::BaseWeakPtr get_base();

    /// Filter ModuleItems by their `name`.
    ///
    /// @param pattern          A string the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_name(const std::string & pattern, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter ModuleItems by their `name`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_name(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter ModuleItems by their `stream`.
    ///
    /// @param pattern          A string the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_stream(const std::string & pattern, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter ModuleItems by their `stream`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_stream(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter ModuleItems by their `version`.
    ///
    /// @param pattern          A string the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_version(const std::string & pattern, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter ModuleItems by their `version`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_version(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter ModuleItems by their `context`.
    ///
    /// @param pattern          A string the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_context(const std::string & pattern, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter ModuleItems by their `context`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_context(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter ModuleItems by their `arch`.
    ///
    /// @param pattern          A string the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_arch(const std::string & pattern, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Filter ModuleItems by their `arch`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp_type         A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_arch(
        const std::vector<std::string> & patterns, libdnf5::sack::QueryCmp cmp_type = libdnf5::sack::QueryCmp::EQ);

    /// Group ModuleItems by `name`, `stream`, `context` and `arch`. Then within each group, keep packages that correspond with up to `limit` of (all but) latest `version`s in the group.
    ///
    /// @param limit            If `limit` > 0, keep `limit` number `version`s in each group.
    ///                         If `limit` < 0, keep all **but** `limit` last `version`s in each group.
    /// @since 5.0.4
    void filter_latest(int limit = 1);

    /// Filter ModuleItems by Nsvcap object.
    ///
    /// @param nsvcap           A Nsvcap object the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0.6
    void filter_nsvca(const Nsvcap & nsvcap, libdnf5::sack::QueryCmp cmp = libdnf5::sack::QueryCmp::EQ);

    /// Filter ModuleItems with ModuleStatus::ENABLED.
    ///
    /// @since 5.1.5
    void filter_enabled();

    /// Filter ModuleItems with ModuleStatus::DISABLED.
    ///
    /// @since 5.1.5
    void filter_disabled();

    /// Filter ModuleItems by module_spec.
    ///
    /// @param module_spec      A module_spec the filter is matched against.
    /// @return                 `true` and matched Nsvcap if the module_spec was parsed successfully,
    ///                         `false` and empty Nsvcap otherwise.
    /// @since 5.0.6
    std::pair<bool, Nsvcap> resolve_module_spec(const std::string & module_spec);

private:
    friend ModuleItem;

    class Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::module

#endif  // LIBDNF5_MODULE_MODULE_QUERY_HPP
