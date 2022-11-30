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

#ifndef LIBDNF_MODULE_MODULE_QUERY_HPP
#define LIBDNF_MODULE_MODULE_QUERY_HPP

#include "libdnf/base/base_weak.hpp"
#include "libdnf/common/sack/query.hpp"
#include "libdnf/common/sack/query_cmp.hpp"
#include "libdnf/common/weak_ptr.hpp"
#include "libdnf/module/module_item.hpp"

#include <string>
#include <vector>


namespace libdnf::module {

class ModuleQuery : public libdnf::sack::Query<ModuleItem> {
public:
    /// Create a new ModuleQuery instance.
    ///
    /// @param base     A weak pointer to Base
    explicit ModuleQuery(const libdnf::BaseWeakPtr & base, bool empty = false);

    /// Create a new ModuleQuery instance.
    ///
    /// @param base     Reference to Base
    explicit ModuleQuery(libdnf::Base & base, bool empty = false);

    /// @return Weak pointer to the Base object.
    /// @since 5.0
    libdnf::BaseWeakPtr get_base() { return base; }

    /// Filter ModuleItems by their `name`.
    ///
    /// @param pattern          A string the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_name(const std::string & pattern, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter ModuleItems by their `name`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_name(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter ModuleItems by their `stream`.
    ///
    /// @param pattern          A string the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_stream(const std::string & pattern, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter ModuleItems by their `stream`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_stream(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter ModuleItems by their `version`.
    ///
    /// @param pattern          A string the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_version(const std::string & pattern, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter ModuleItems by their `version`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_version(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter ModuleItems by their `context`.
    ///
    /// @param pattern          A string the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_context(const std::string & pattern, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter ModuleItems by their `context`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_context(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter ModuleItems by their `arch`.
    ///
    /// @param pattern          A string the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_arch(const std::string & pattern, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

    /// Filter ModuleItems by their `arch`.
    ///
    /// @param patterns         A vector of strings the filter is matched against.
    /// @param cmp              A comparison (match) operator, defaults to `QueryCmp::EQ`.
    ///                         Supported values: `EQ`, `NEQ`, `GLOB`, `NOT_GLOB`, `IEXACT`, `NOT_IEXACT`, `ICONTAINS`, `NOT_ICONTAINS`, `IGLOB`, `NOT_IGLOB`, `CONTAINS`, `NOT_CONTAINS`.
    /// @since 5.0
    void filter_arch(
        const std::vector<std::string> & patterns, libdnf::sack::QueryCmp cmp_type = libdnf::sack::QueryCmp::EQ);

private:
    // Getter callbacks that return attribute values from an object. Used in query filters.
    struct Get {
        static std::string name(const ModuleItem & obj) { return obj.get_name(); }
        static std::string stream(const ModuleItem & obj) { return obj.get_stream(); }
        static std::string version(const ModuleItem & obj) { return obj.get_version_str(); }
        static std::string context(const ModuleItem & obj) { return obj.get_context(); }
        static std::string arch(const ModuleItem & obj) { return obj.get_arch(); }
    };

    friend ModuleItem;

    BaseWeakPtr base;
};

}  // namespace libdnf::module

#endif  // LIBDNF_MODULE_MODULE_QUERY_HPP
