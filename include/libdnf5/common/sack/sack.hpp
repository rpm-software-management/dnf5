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

#ifndef LIBDNF5_COMMON_SACK_SACK_HPP
#define LIBDNF5_COMMON_SACK_SACK_HPP

#include "libdnf5/common/set.hpp"
#include "libdnf5/common/weak_ptr.hpp"

#include <memory>
#include <vector>


namespace libdnf5::sack {

template <typename T>
class Sack {
public:
    using DataItemWeakPtr = WeakPtr<T, false>;

    std::size_t size() const noexcept { return data.size(); }

    // EXCLUDES

    const Set<DataItemWeakPtr> & get_excludes() const noexcept { return excludes; }
    void add_excludes(const Set<DataItemWeakPtr> & value) { excludes.update(value); }
    void remove_excludes(const Set<DataItemWeakPtr> & value) { excludes.difference(value); }
    void set_excludes(const Set<DataItemWeakPtr> & value) { excludes = value; }

    // INCLUDES

    const Set<DataItemWeakPtr> & get_includes() const noexcept { return includes; }
    void add_includes(const Set<DataItemWeakPtr> & value) { includes.update(value); }
    void remove_includes(const Set<DataItemWeakPtr> & value) { includes.difference(value); }
    void set_includes(const Set<DataItemWeakPtr> & value) { includes = value; }
    bool get_use_includes() const noexcept { return use_includes; }
    void set_use_includes(bool value) { use_includes = value; }

protected:
    Sack() = default;
    DataItemWeakPtr add_item_with_return(std::unique_ptr<T> && item);
    void add_item(std::unique_ptr<T> && item);
    std::vector<std::unique_ptr<T>> & get_data() { return data; }
    WeakPtrGuard<T, false> & get_data_guard() { return data_guard; }

private:
    WeakPtrGuard<T, false> data_guard;
    Set<DataItemWeakPtr> excludes;
    Set<DataItemWeakPtr> includes;
    bool use_includes = false;
    std::vector<std::unique_ptr<T>> data;  // Owns the data set. Objects get deleted when the Sack is deleted.
};

template <typename T>
typename Sack<T>::DataItemWeakPtr Sack<T>::add_item_with_return(std::unique_ptr<T> && item) {
    auto ret = DataItemWeakPtr(item.get(), &data_guard);
    data.push_back(std::move(item));
    return ret;
}

template <typename T>
void Sack<T>::add_item(std::unique_ptr<T> && item) {
    data.push_back(std::move(item));
}

}  // namespace libdnf5::sack

#endif
