/*
 * Copyright (C) 2019 Red Hat, Inc.
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _LIBDNF_PRESERVE_ORDER_MAP_HPP
#define _LIBDNF_PRESERVE_ORDER_MAP_HPP

#ifdef LIBDNF_UNSTABLE_API

#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <vector>

namespace libdnf {

template<typename Key, typename T, class KeyEqual = std::equal_to<Key>>
class PreserveOrderMap {
public:
    typedef Key key_type;
    typedef T mapped_type;
    typedef KeyEqual key_equal;
    typedef std::pair<const Key, T> value_type;
    typedef std::vector<std::pair<Key, T>> container_type;
    typedef typename container_type::size_type size_type;

    template<typename valueType, typename ContainerTypeIterator>
    struct MyBidirIterator {
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef typename PreserveOrderMap::value_type value_type;
        typedef ptrdiff_t difference_type;
        typedef valueType * pointer;
        typedef valueType & reference;

        explicit MyBidirIterator() = default;
        explicit MyBidirIterator(ContainerTypeIterator ci) : ci(ci) {}

        reference operator *() { return reinterpret_cast<reference>(*ci); }
        pointer operator ->() const { return reinterpret_cast<pointer>(ci.operator->()); }

        MyBidirIterator& operator ++() { ++ci; return *this; }
        MyBidirIterator operator ++(int) { auto tmp = *this; ++*this; return tmp; }
        MyBidirIterator& operator --() { --ci; return *this; }
        MyBidirIterator operator --(int) { auto tmp = *this; --*this; return tmp; }

        bool operator ==(const MyBidirIterator& other) const { return ci == other.ci; }
        bool operator !=(const MyBidirIterator& other) const { return ci != other.ci; }

    private:
        friend class PreserveOrderMap;
        ContainerTypeIterator ci;
    };
    typedef MyBidirIterator<value_type, typename container_type::iterator> iterator;
    typedef MyBidirIterator<const value_type, typename container_type::const_iterator> const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    bool empty() const noexcept { return items.empty(); }
    size_type size() const noexcept { return items.size(); }
    size_type max_size() const noexcept { return items.max_size(); }
    void reserve(size_type newCapacity) { items.reserve(newCapacity); }
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

    std::pair<iterator, bool> insert(const value_type & value)
    {
        auto it = find(value.first);
        if (it == end()) {
            it = iterator(items.insert(it.ci, value));
            return {it, true};
        } else {
            return {it, false};
        }
    }

    iterator erase(const_iterator pos) { return iterator(items.erase(pos.ci)); }
    iterator erase(const_iterator first, const_iterator last) { return iterator(items.erase(first.ci, last.ci)); }

    size_type erase(const Key & key)
    {
        auto it = find(key);
        if (it == end())
            return 0;
        items.erase(it.ci);
        return 1;
    }

    size_type count(const Key & key) const
    {
        return find(key) != end() ? 1 : 0;
    }

    iterator find(const Key & key)
    {
        auto it = begin();
        while (it != end() && !KeyEqual()(it->first, key))
            ++it;
        return it;
    }

    const_iterator find(const Key & key) const
    {
        auto it = cbegin();
        while (it != cend() && !KeyEqual()(it->first, key))
            ++it;
        return it;
    }

    T & operator[](const Key & key) {
        for (auto & item : items) {
            if (KeyEqual()(item.first, key))
                return item.second;
        }
        items.push_back({key, {}});
        return items.back().second;
    }

    T & operator[](Key && key)
    {
        for (auto & item : items) {
            if (KeyEqual()(item.first, key))
                return item.second;
        }
        items.push_back({std::move(key), {}});
        return items.back().second;
    }

    T & at(const Key & key)
    {
        for (auto & item : items) {
            if (KeyEqual()(item.first, key))
                return item.second;
        }
        throw std::out_of_range("PreserveOrderMap::at");
    }

    const T & at(const Key & key) const
    {
        for (auto & item : items) {
            if (KeyEqual()(item.first, key))
                return item.second;
        }
        throw std::out_of_range("PreserveOrderMap::at");
    }

private:
    container_type items;
};

}

#endif

#endif //_LIBDNF_PRESERVE_ORDER_MAP_HPP
