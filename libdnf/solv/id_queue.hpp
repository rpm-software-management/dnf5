/*
Copyright (C) 2017-2020 Red Hat, Inc.

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

#ifndef LIBDNF_SOLV_ID_QUEUE_HPP
#define LIBDNF_SOLV_ID_QUEUE_HPP

#include "queue_iterator.hpp"
#include "solv_map.hpp"

#include <utility>

extern "C" {
#include <solv/queue.h>
}


namespace libdnf::solv {

struct IdQueue {
public:
    using iterator = IdQueueIterator;

    iterator begin() const { return iterator(&queue); }

    iterator end() const {
        iterator it(&queue);
        it.end();
        return it;
    }

    IdQueue() { queue_init(&queue); }

    IdQueue(const IdQueue & src) { queue_init_clone(&queue, &src.queue); }

    IdQueue(IdQueue && src) {
        queue_init(&queue);
        std::swap(queue, src.queue);
    }

    explicit IdQueue(const Queue & src) { queue_init_clone(&queue, &src); }

    ~IdQueue() { queue_free(&queue); }

    bool operator==(const IdQueue & other) const;

    bool operator!=(const IdQueue & other) const { return !(*this == other); }

    IdQueue & operator=(const IdQueue & src);

    IdQueue & operator=(IdQueue && src) noexcept {
        std::swap(queue, src.queue);
        return *this;
    }

    IdQueue & operator+=(const IdQueue & src) {
        append(src);
        return *this;
    }

    IdQueue & operator+=(Id id) {
        push_back(id);
        return *this;
    }

    Id operator[](int index) const { return queue.elements[index]; }

    void push_back(Id id) { queue_push(&queue, id); }

    /// @brief Add two Ids into queue.
    /// It is used by libsolv jobs when one Id represents what to perform and the second one on witch elements
    void push_back(Id id1, Id id2) { queue_push2(&queue, id1, id2); }

    int * data() const noexcept { return queue.elements; }

    Queue & get_queue() noexcept { return queue; }

    const Queue & get_queue() const noexcept { return queue; }

    bool empty() const noexcept { return queue.count == 0; };

    int size() const noexcept { return queue.count; }

    void clear() noexcept { queue_empty(&queue); }

    void append(const IdQueue & src) { queue_insertn(&queue, size(), src.size(), src.data()); }

    void reserve(int n) { queue_prealloc(&queue, n); }

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
    append(src);
    return *this;
}


void inline solv_map_to_id_queue(IdQueue & ids, const SolvMap & src) {
    ids.clear();
    for (auto solv_id : src) {
        ids.push_back(solv_id);
    }
}

}  // namespace libdnf::solv

#endif  // LIBDNF_SOLV_ID_QUEUE_HPP
