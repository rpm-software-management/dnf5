/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef LIBDNF_SOLV_MAP_HPP
#define LIBDNF_SOLV_MAP_HPP


#include <solv/bitmap.h>
#include <solv/pooltypes.h>

#include <iterator>


namespace libdnf::solv {

// clang-format off
// see http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetTable
static const unsigned char BIT_COUNT_LOOKUP[256] = {
    #define B2(n) n, n + 1, n + 1, n + 2
    #define B4(n) B2(n), B2(n + 1), B2(n + 1), B2(n + 2)
    #define B6(n) B4(n), B4(n + 1), B4(n + 1), B4(n + 2)
    B6(0), B6(1), B6(1), B6(2)
};
// clang-format on


class SolvMapIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Id;
    using pointer = void;
    using reference = Id;

    explicit SolvMapIterator(const Map * map) noexcept : map{map}, map_end{map->map + map->size} { begin(); }

    SolvMapIterator(const SolvMapIterator & other) noexcept = default;

    Id operator*() const noexcept { return current_value; }

    SolvMapIterator & operator++() noexcept;

    SolvMapIterator operator++(int) noexcept {
        SolvMapIterator it(*this);
        ++*this;
        return it;
    }

    bool operator==(const SolvMapIterator & other) const noexcept { return current_value == other.current_value; }

    bool operator!=(const SolvMapIterator & other) const noexcept { return current_value != other.current_value; }

    void begin() noexcept {
        current_value = BEGIN;
        map_current = map->map;
        ++*this;
    }

    void end() noexcept {
        current_value = END;
        map_current = map_end;
    }

    /// Sets iterator to the first contained package in the range <id, end>.
    void jump(Id id) noexcept;

protected:
    const Map * get_map() const noexcept { return map; }

private:
    constexpr static int BEGIN = -1;
    constexpr static int END = -2;

    // pointer to a map owned by SolvMap
    const Map * map;

    // current address in the map
    const unsigned char * map_current;

    // the last address in the map
    const unsigned char * map_end;

    // value of the iterator
    Id current_value;
};


class SolvMap {
public:
    using iterator = SolvMapIterator;

    explicit SolvMap(int size) { map_init(&map, size); }

    SolvMap(const SolvMap & other) : SolvMap(other.get_map()) {}

    /// Clones from an existing libsolv Map.
    explicit SolvMap(const Map * map) { map_init_clone(&this->map, map); }

    SolvMap(SolvMap && other) noexcept {
        map = other.map;
        other.map.map = nullptr;
        other.map.size = 0;
    }

    ~SolvMap() { map_free(&map); }

    SolvMap & operator=(const SolvMap & other) noexcept;
    SolvMap & operator=(SolvMap && other) noexcept;

    iterator begin() const { return iterator(&map); }

    iterator end() const {
        iterator it(&map);
        it.end();
        return it;
    }

    // GENERIC OPERATIONS

    /// Grows the map to a bigger size.
    ///
    /// @param size The new size to grow to.
    void grow(int size) { map_grow(&map, size); };

    /// Sets all bits in the map to 1.
    void set_all() { map_setall(&map); };

    /// Sets all bits in the map to 0.
    void clear() noexcept { map_empty(&map); }

    const Map * get_map() const noexcept { return &map; }

    /// @return the number of solvables in the SolvMap (number of 1s in the bitmap).
    std::size_t size() const noexcept;

    /// @return the size allocated for the map in memory (in number of items, not bytes).
    int allocated_size() const noexcept { return map.size << 3; }

    /// @return whether the map is empty.
    bool empty() const noexcept;

    // ITEM OPERATIONS

    void add(Id id) {
        check_id_in_bitmap_range(id);
        add_unsafe(id);
    }

    void add_unsafe(Id id) noexcept;

    bool contains(Id id) const noexcept;

    bool contains_unsafe(Id id) const noexcept { return MAPTST(&map, id); }

    void remove(Id id) {
        check_id_in_bitmap_range(id);
        remove_unsafe(id);
    }

    void remove_unsafe(Id id) noexcept;

    // SET OPERATIONS - Map

    /// Union operator
    SolvMap & operator|=(const Map * other) noexcept {
        map_or(&map, const_cast<Map *>(other));
        return *this;
    }

    /// Difference operator
    SolvMap & operator-=(const Map * other) noexcept {
        map_subtract(&map, const_cast<Map *>(other));
        return *this;
    }

    /// Intersection operator
    SolvMap & operator&=(const Map * other) noexcept {
        map_and(&map, other);
        return *this;
    }

    // SET OPERATIONS - SolvMap

    /// Union operator
    SolvMap & operator|=(const SolvMap & other) noexcept {
        *this |= other.get_map();
        return *this;
    }

    /// Difference operator
    SolvMap & operator-=(const SolvMap & other) noexcept {
        *this -= other.get_map();
        return *this;
    }

    /// Intersection operator
    SolvMap & operator&=(const SolvMap & other) noexcept {
        *this &= other.get_map();
        return *this;
    }

    /// Swaps the underlying libsolv Map pointers.
    void swap(SolvMap & other) noexcept { std::swap(map, other.map); }

protected:
    /// Check if `id` is in bitmap range.
    ///
    /// @exception std::out_of_range `id` is out of the range of the bitmap.
    void check_id_in_bitmap_range(Id id) const;

private:
    Map map;
};


inline SolvMapIterator & SolvMapIterator::operator++() noexcept {
    if (current_value >= 0) {
        // make a copy of byte with the previous match to avoid changing the map
        unsigned char byte = *map_current;

        // reset previously seen bits to 0
        byte >>= (current_value & 7) + 1;

        auto bit = ffs(byte << ((current_value & 7) + 1));
        if (bit) {
            // return (current byte * 8) + bit position - 1
            current_value = (static_cast<int>(map_current - map->map) << 3) + bit - 1;
            return *this;
        }

        // no following bit was set in the current byte
        // move to the next byte and start searching in the next bytes
        map_current++;
    }

    while (map_current < map_end) {
        // skip all empty bytes
        if (!*map_current) {
            // move to the next byte
            map_current++;
            continue;
        }

        // now we have a byte that has at least one bit set
        // return (current byte * 8) + bit position - 1
        current_value = (static_cast<int>(map_current - map->map) << 3) + ffs(*map_current) - 1;
        return *this;
    }

    // not found
    current_value = END;
    return *this;
}


inline void SolvMapIterator::jump(Id id) noexcept {
    if (id < 0) {
        begin();
        return;
    }

    const unsigned char * current = map->map + (id >> 3);

    if (current >= map_end) {
        end();
        return;
    }

    current_value = id;
    map_current = current;

    // If the element with requested id does not exist in the map, it moves to the next.
    if (!(*current & (1 << (id & 7)))) {
        ++*this;
    }
}


inline SolvMap & SolvMap::operator=(const SolvMap & other) noexcept {
    if (this != &other) {
        if (map.size == other.map.size) {
            memcpy(map.map, other.map.map, static_cast<size_t>(map.size));
        } else {
            map_free(&map);
            map_init_clone(&map, &other.map);
        }
    }
    return *this;
}


inline SolvMap & SolvMap::operator=(SolvMap && other) noexcept {
    if (this != &other) {
        map = other.map;
        other.map.map = nullptr;
        other.map.size = 0;
    }
    return *this;
}


inline void SolvMap::check_id_in_bitmap_range(Id id) const {
    if (id < 0 || id >= allocated_size()) {
        throw std::out_of_range("Id is out of bitmap range");
    }
}


inline bool SolvMap::contains(Id id) const noexcept {
    if (id < 0 || id >= allocated_size()) {
        // if Id is outside bitmap range, then bitmap doesn't contain it
        return false;
    }
    return contains_unsafe(id);
}


inline void SolvMap::add_unsafe(Id id) noexcept {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    MAPSET(&map, id);
#pragma GCC diagnostic pop
}


inline void SolvMap::remove_unsafe(Id id) noexcept {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    MAPCLR(&map, id);
#pragma GCC diagnostic pop
}


inline bool SolvMap::empty() const noexcept {
    const unsigned char * byte = map.map;
    const unsigned char * end = byte + map.size;

    // iterate through the whole bitmap by moving the address
    while (byte < end) {
        if (*byte++) {
            // return false if a non-zero bit was found
            return false;
        }
    }
    // all bits were zero, return true
    return true;
}


inline std::size_t SolvMap::size() const noexcept {
    unsigned char * byte = map.map;
    unsigned char * end = byte + map.size;
    std::size_t result = 0;

    // iterate through the whole bitmap by moving the address
    while (byte < end) {
        // add number of bits in each byte
        result += BIT_COUNT_LOOKUP[*byte++];
    }
    return result;
}

}  // namespace libdnf::solv

#endif  // LIBDNF_SOLV_MAP_HPP
