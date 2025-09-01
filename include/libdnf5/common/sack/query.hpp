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

#ifndef LIBDNF5_COMMON_SACK_QUERY_HPP
#define LIBDNF5_COMMON_SACK_QUERY_HPP

#include "match_int64.hpp"
#include "match_string.hpp"
#include "query_cmp.hpp"

#include "libdnf5/common/exception.hpp"
#include "libdnf5/common/set.hpp"
#include "libdnf5/defs.h"

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>


namespace libdnf5::sack {

LIBDNF_API extern const BgettextMessage msg_err_exact_one_object;

/// Query is a Set with filtering capabilities.
template <typename T>
class Query : public Set<T> {
public:
    using FilterFunctionBool = bool(const T & obj);
    using FilterFunctionCString = char *(const T & obj);
    using FilterFunctionInt64 = int64_t(const T & obj);
    using FilterFunctionString = std::string(const T & obj);
    using FilterFunctionVectorInt64 = std::vector<int64_t>(const T & obj);
    using FilterFunctionVectorString = std::vector<std::string>(const T & obj);

    Query() = default;
    explicit Query(const Set<T> & src_set) : Set<T>::Set(src_set) {}
    explicit Query(Set<T> && src_set) : Set<T>::Set(std::move(src_set)) {}

    void filter(std::string (*getter)(const T &), const std::string & pattern, QueryCmp cmp);
    void filter(std::vector<std::string> (*getter)(const T &), const std::string & pattern, QueryCmp cmp);
    void filter(std::string (*getter)(const T &), const std::vector<std::string> & patterns, QueryCmp cmp);
    void filter(std::vector<std::string> (*getter)(const T &), const std::vector<std::string> & patterns, QueryCmp cmp);

    void filter(int64_t (*getter)(const T &), int64_t pattern, QueryCmp cmp);
    void filter(std::vector<int64_t> (*getter)(const T &), int64_t pattern, QueryCmp cmp);
    void filter(int64_t (*getter)(const T &), const std::vector<int64_t> & patterns, QueryCmp cmp);
    void filter(std::vector<int64_t> (*getter)(const T &), const std::vector<int64_t> & patterns, QueryCmp cmp);

    void filter(bool (*getter)(const T &), bool pattern, QueryCmp cmp);

    void filter(char * (*getter)(const T &), const std::string & pattern, QueryCmp cmp);

    /// Get a single object. Raise an exception if none or multiple objects match the query.
    const T & get() const {
        if (get_data().size() == 1) {
            return *get_data().begin();
        }
        throw RuntimeError(msg_err_exact_one_object);
    }

    /// List all objects matching the query.
    const std::set<T> & list() const noexcept { return get_data(); }

    // operators; OR at least
    // copy()
    using Set<T>::get_data;

private:
    static void throw_except_one_object();
};


template <typename T>
inline void Query<T>::filter(Query<T>::FilterFunctionString * getter, const std::string & pattern, QueryCmp cmp) {
    for (auto it = get_data().begin(); it != get_data().end();) {
        auto value = getter(*it);
        if (match_string(value, cmp, pattern)) {
            ++it;
        } else {
            // TODO(jrohel): Modifying of iterated dataset. Performance? Can be implement better?
            it = get_data().erase(it);
        }
    }
}


template <typename T>
inline void Query<T>::filter(Query<T>::FilterFunctionVectorString * getter, const std::string & pattern, QueryCmp cmp) {
    for (auto it = get_data().begin(); it != get_data().end();) {
        auto values = getter(*it);
        if (match_string(values, cmp, pattern)) {
            ++it;
        } else {
            it = get_data().erase(it);
        }
    }
}

template <typename T>
inline void Query<T>::filter(
    Query<T>::FilterFunctionString * getter, const std::vector<std::string> & patterns, QueryCmp cmp) {
    for (auto it = get_data().begin(); it != get_data().end();) {
        auto value = getter(*it);
        if (match_string(value, cmp, patterns)) {
            ++it;
        } else {
            it = get_data().erase(it);
        }
    }
}

template <typename T>
inline void Query<T>::filter(
    Query<T>::FilterFunctionVectorString * getter, const std::vector<std::string> & patterns, QueryCmp cmp) {
    for (auto it = get_data().begin(); it != get_data().end();) {
        auto values = getter(*it);
        if (match_string(values, cmp, patterns)) {
            ++it;
        } else {
            it = get_data().erase(it);
        }
    }
}

template <typename T>
inline void Query<T>::filter(FilterFunctionInt64 * getter, int64_t pattern, QueryCmp cmp) {
    for (auto it = get_data().begin(); it != get_data().end();) {
        auto value = getter(*it);
        if (match_int64(value, cmp, pattern)) {
            ++it;
        } else {
            it = get_data().erase(it);
        }
    }
}

template <typename T>
inline void Query<T>::filter(FilterFunctionVectorInt64 * getter, int64_t pattern, QueryCmp cmp) {
    for (auto it = get_data().begin(); it != get_data().end();) {
        auto values = getter(*it);
        if (match_int64(values, cmp, pattern)) {
            ++it;
        } else {
            it = get_data().erase(it);
        }
    }
}

template <typename T>
inline void Query<T>::filter(FilterFunctionInt64 * getter, const std::vector<int64_t> & patterns, QueryCmp cmp) {
    for (auto it = get_data().begin(); it != get_data().end();) {
        auto value = getter(*it);
        if (match_int64(value, cmp, patterns)) {
            ++it;
        } else {
            it = get_data().erase(it);
        }
    }
}

template <typename T>
inline void Query<T>::filter(FilterFunctionVectorInt64 * getter, const std::vector<int64_t> & patterns, QueryCmp cmp) {
    for (auto it = get_data().begin(); it != get_data().end();) {
        auto values = getter(*it);
        if (match_int64(values, cmp, patterns)) {
            ++it;
        } else {
            it = get_data().erase(it);
        }
    }
}

// TODO: other cmp
template <typename T>
inline void Query<T>::filter(Query<T>::FilterFunctionBool * getter, bool pattern, QueryCmp cmp) {
    for (auto it = get_data().begin(); it != get_data().end();) {
        auto value = getter(*it);
        if (cmp == QueryCmp::EQ && value == pattern) {
            ++it;
        } else {
            it = get_data().erase(it);
        }
    }
}

template <typename T>
inline void Query<T>::filter(Query<T>::FilterFunctionCString * getter, const std::string & pattern, QueryCmp cmp) {
    for (auto it = get_data().begin(); it != get_data().end();) {
        auto value = getter(*it);
        if (match_string(value, cmp, pattern)) {
            ++it;
        } else {
            it = get_data().erase(it);
        }
    }
}

}  // namespace libdnf5::sack

#endif
