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

#ifndef LIBDNF5_SOLV_ID_QUEUE_HPP
#define LIBDNF5_SOLV_ID_QUEUE_HPP

#include "solv_map.hpp"

#include <utility>

extern "C" {
#include <solv/queue.h>
#include <solv/util.h>
}


namespace libdnf5::solv {

class IdQueueIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Id;
    using pointer = void;
    using reference = Id;

    explicit IdQueueIterator(const Queue * queue) noexcept : queue{queue} { begin(); }

    IdQueueIterator(const IdQueueIterator & other) noexcept = default;

    Id & operator*() noexcept { return queue->elements[current_index]; }

    Id operator*() const noexcept { return queue->elements[current_index]; }

    IdQueueIterator & operator++() noexcept {
        ++current_index;
        return *this;
    }

    IdQueueIterator operator++(int) noexcept {
        IdQueueIterator ret(*this);
        ++*this;
        return ret;
    }

    bool operator==(const IdQueueIterator & other) const noexcept { return current_index == other.current_index; }

    bool operator!=(const IdQueueIterator & other) const noexcept { return current_index != other.current_index; }

    void begin() noexcept { current_index = 0; }

    void end() noexcept { current_index = queue->count; }

protected:
    const Queue * get_queue() const noexcept { return queue; }

private:
    const Queue * queue;
    int current_index;
};


struct IdQueue {
public:
    using iterator = IdQueueIterator;

    IdQueue() { queue_init(&queue); }

    IdQueue(const IdQueue & src) { queue_init_clone(&queue, &src.queue); }

    IdQueue(IdQueue && src) {
        queue_init(&queue);
        std::swap(queue, src.queue);
    }

    explicit IdQueue(const Queue & src) { queue_init_clone(&queue, &src); }

    ~IdQueue() { queue_free(&queue); }

    IdQueue & operator=(const IdQueue & src);

    IdQueue & operator=(IdQueue && src) noexcept {
        std::swap(queue, src.queue);
        return *this;
    }

    iterator begin() const { return iterator(&queue); }

    iterator end() const {
        iterator it(&queue);
        it.end();
        return it;
    }

    bool operator==(const IdQueue & other) const;

    bool operator!=(const IdQueue & other) const { return !(*this == other); }

    IdQueue & operator+=(const IdQueue & src) {
        queue_insertn(&queue, size(), src.size(), src.queue.elements);
        return *this;
    }

    Id & operator[](int index) { return queue.elements[index]; }

    Id operator[](int index) const { return queue.elements[index]; }

    void push_back(Id id) { queue_push(&queue, id); }

    /// Adds two Ids into the queue.
    /// Used in libsolv jobs which work with pairs (operation, item).
    void push_back(Id id1, Id id2) { queue_push2(&queue, id1, id2); }

    Queue & get_queue() noexcept { return queue; }

    const Queue & get_queue() const noexcept { return queue; }

    /// @return Whether the queue is empty.
    bool empty() const noexcept { return queue.count == 0; };

    /// @return The number of items in the queue.
    int size() const noexcept { return queue.count; }

    /// Clears the queue.
    void clear() noexcept { queue_empty(&queue); }

    /// Allocates space for `n` more Ids in the queue (the resulting size is `size() + n`).
    void reserve(int n) { queue_prealloc(&queue, n); }

    /// Sorts the queue using comparator `cmp`. The comparator should return 0
    /// if the elements are the same, -1 if `a` sorts before `b` and 1 if `b`
    /// sorts before `a`.
    /// @param cmp The comparator function. `a` and `b` are pointers to `Id`s
    ///            to compare, `data` is the `data` pointer passed to this method.
    /// @param data Any data required for the comparison, the pointer will be passed to `cmp`.
    template <typename TData>
    void sort(int (*cmp)(const Id * a, const Id * b, TData * data), TData * data);

private:
    Queue queue;
};


inline bool IdQueue::operator==(const IdQueue & other) const {
    if (size() != other.size()) {
        return false;
    }

    for (int i = 0; i < size(); i++) {
        if ((*this)[i] != other[i]) {
            return false;
        }
    }
    return true;
}


inline IdQueue & IdQueue::operator=(const IdQueue & src) {
    if (this == &src) {
        return *this;
    }
    queue_empty(&queue);
    *this += src;
    return *this;
}


void inline solv_map_to_id_queue(IdQueue & ids, const SolvMap & src) {
    ids.clear();
    for (auto solv_id : src) {
        ids.push_back(solv_id);
    }
}

template <typename TData>
inline void IdQueue::sort(int (*cmp)(const Id * a, const Id * b, TData * data), TData * data) {
    solv_sort(
        queue.elements,
        static_cast<size_t>(size()),
        sizeof(Id),
        (int (*)(const void * a, const void * b, void * data))cmp,
        (void *)data);
}

}  // namespace libdnf5::solv

#endif  // LIBDNF5_SOLV_ID_QUEUE_HPP
