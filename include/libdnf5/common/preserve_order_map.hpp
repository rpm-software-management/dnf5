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

#ifndef LIBDNF5_UTILS_PRESERVE_ORDER_MAP_HPP
#define LIBDNF5_UTILS_PRESERVE_ORDER_MAP_HPP

#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <vector>


namespace libdnf5 {

/// PreserveOrderMap is an associative container that contains key-value pairs with unique unique keys.
/// It is similar to standard std::map. But it preserves the order of items and the complexity is linear.
template <typename Key, typename T, class KeyEqual = std::equal_to<Key>>
class PreserveOrderMap {
public:
    using key_type = Key;
    using mapped_type = T;
    using key_equal = KeyEqual;
    using value_type = std::pair<const Key, T>;
    using container_type = std::vector<std::pair<Key, T>>;
    using size_type = typename container_type::size_type;

    template <typename valueType, typename ContainerTypeIterator>
    struct BidirIterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = typename PreserveOrderMap::value_type;
        using difference_type = ptrdiff_t;
        using pointer = valueType *;
        using reference = valueType &;

        explicit BidirIterator() = default;
        explicit BidirIterator(ContainerTypeIterator ci) : ci(ci) {}

        // Allow iterator to const_iterator conversion
        template <typename CItType = ContainerTypeIterator>
        BidirIterator(
            const BidirIterator<value_type, typename container_type::iterator> & src,
            typename std::enable_if<std::is_same<CItType, typename container_type::const_iterator>::value>::type * = 0)
            : ci(src.ci) {}

        // Allow reverse_iterator to const_reverse_iterator conversion
        template <typename CItType = ContainerTypeIterator>
        BidirIterator(
            const BidirIterator<value_type, typename container_type::reverse_iterator> & src,
            typename std::enable_if<
                std::is_same<CItType, typename container_type::const_reverse_iterator>::value>::type * = 0)
            : ci(src.ci) {}

        reference operator*() const { return reinterpret_cast<reference>(*ci); }
        pointer operator->() const { return reinterpret_cast<pointer>(ci.operator->()); }

        BidirIterator & operator++() {
            ++ci;
            return *this;
        }
        BidirIterator operator++(int) {
            auto tmp = *this;
            ++*this;
            return tmp;
        }
        BidirIterator & operator--() {
            --ci;
            return *this;
        }
        BidirIterator operator--(int) {
            auto tmp = *this;
            --*this;
            return tmp;
        }

        bool operator==(const BidirIterator & other) const { return ci == other.ci; }
        bool operator!=(const BidirIterator & other) const { return ci != other.ci; }

    private:
        friend class PreserveOrderMap;
        ContainerTypeIterator ci;
    };
    using iterator = BidirIterator<value_type, typename container_type::iterator>;
    using const_iterator = BidirIterator<const value_type, typename container_type::const_iterator>;
    using reverse_iterator = BidirIterator<value_type, typename container_type::reverse_iterator>;
    using const_reverse_iterator = BidirIterator<const value_type, typename container_type::const_reverse_iterator>;

    bool empty() const noexcept { return items.empty(); }
    size_type size() const noexcept { return items.size(); }
    size_type max_size() const noexcept { return items.max_size(); }
    void reserve(size_type new_capacity) { items.reserve(new_capacity); }
    size_type capacity() const noexcept { return items.capacity(); }
    void shrink_to_fit() { items.shrink_to_fit(); }

    iterator begin() noexcept { return iterator(items.begin()); }
    const_iterator begin() const noexcept { return const_iterator(items.begin()); }
    const_iterator cbegin() const noexcept { return const_iterator(items.cbegin()); }
    iterator end() noexcept { return iterator(items.end()); }
    const_iterator end() const noexcept { return const_iterator(items.end()); }
    const_iterator cend() const noexcept { return const_iterator(items.cend()); }
    reverse_iterator rbegin() noexcept { return reverse_iterator(items.rbegin()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(items.rbegin()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(items.crbegin()); }
    reverse_iterator rend() noexcept { return reverse_iterator(items.rend()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(items.rend()); }
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(items.crend()); }

    void clear() noexcept { items.clear(); }

    std::pair<iterator, bool> insert(const value_type & value) {
        auto it = find(value.first);
        if (it == end()) {
            it = iterator(items.insert(it.ci, value));
            return {it, true};
        }
        return {it, false};
    }

    iterator erase(const_iterator pos) { return iterator(items.erase(pos.ci)); }
    iterator erase(iterator pos) { return iterator(items.erase(pos.ci)); }
    iterator erase(const_iterator first, const_iterator last) { return iterator(items.erase(first.ci, last.ci)); }

    size_type erase(const Key & key) {
        auto it = find(key);
        if (it == end()) {
            return 0;
        }
        items.erase(it.ci);
        return 1;
    }

    size_type count(const Key & key) const { return find(key) != end() ? 1 : 0; }

    iterator find(const Key & key) {
        auto it = begin();
        while (it != end() && !KeyEqual()(it->first, key)) {
            ++it;
        }
        return it;
    }

    const_iterator find(const Key & key) const {
        auto it = cbegin();
        while (it != cend() && !KeyEqual()(it->first, key)) {
            ++it;
        }
        return it;
    }

    T & operator[](const Key & key) {
        for (auto & item : items) {
            if (KeyEqual()(item.first, key)) {
                return item.second;
            }
        }
        items.push_back({key, {}});
        return items.back().second;
    }

    T & operator[](Key && key) {
        for (auto & item : items) {
            if (KeyEqual()(item.first, key)) {
                return item.second;
            }
        }
        items.push_back({std::move(key), {}});
        return items.back().second;
    }

    T & at(const Key & key) {
        for (auto & item : items) {
            if (KeyEqual()(item.first, key)) {
                return item.second;
            }
        }
        throw std::out_of_range("PreserveOrderMap::at");
    }

    const T & at(const Key & key) const {
        for (auto & item : items) {
            if (KeyEqual()(item.first, key)) {
                return item.second;
            }
        }
        throw std::out_of_range("PreserveOrderMap::at");
    }

private:
    container_type items;
};

}  // namespace libdnf5

#endif
