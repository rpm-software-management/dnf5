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

#ifndef LIBDNF_RPM_SOLV_ID_QUEUE_HPP
#define LIBDNF_RPM_SOLV_ID_QUEUE_HPP

#include <utility>

extern "C" {
#include <solv/queue.h>
}

namespace libdnf::rpm::solv {

struct IdQueue {
public:
    IdQueue();
    IdQueue(const IdQueue & src);
    IdQueue(IdQueue && src);
    explicit IdQueue(const Queue & src);
    ~IdQueue();

    bool operator==(const IdQueue & other) const;
    bool operator!=(const IdQueue & other) const;
    IdQueue & operator=(const IdQueue & src);
    IdQueue & operator=(IdQueue && src) noexcept;
    IdQueue & operator+=(const IdQueue & src);
    IdQueue & operator+=(Id id);
    Id operator[](int index) const;

    void push_back(Id id);

    /// @brief Add two Ids into queue.
    /// It is used by libsolv jobs when one Id represents what to perform and the second one on witch elements
    void push_back(Id id1, Id id2);
    int * data() const noexcept;
    Queue & get_queue() noexcept;
    bool empty() const noexcept { return queue.count == 0; };
    int size() const noexcept;
    void clear() noexcept;
    void append(const IdQueue & src);
    void reserve(int n);

private:
    Queue queue;
};

inline IdQueue::IdQueue() {
    queue_init(&queue);
}
inline IdQueue::IdQueue(const IdQueue & src) {
    queue_init_clone(&queue, &src.queue);
}
inline IdQueue::IdQueue(IdQueue && src) {
    queue_init(&queue);
    std::swap(queue, src.queue);
}
inline IdQueue::IdQueue(const Queue & src) {
    queue_init_clone(&queue, &src);
}

inline IdQueue::~IdQueue() {
    queue_free(&queue);
}

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

inline bool IdQueue::operator!=(const IdQueue & other) const {
    return !(*this == other);
}

inline IdQueue & IdQueue::operator=(const IdQueue & src) {
    if (this == &src) {
        return *this;
    }
    queue_empty(&queue);
    append(src);
    return *this;
}

inline IdQueue & IdQueue::operator=(IdQueue && src) noexcept {
    std::swap(queue, src.queue);
    return *this;
}

inline IdQueue & IdQueue::operator+=(const IdQueue & src) {
    append(src);
    return *this;
}

inline IdQueue & IdQueue::operator+=(Id id) {
    push_back(id);
    return *this;
}

inline void IdQueue::push_back(Id id) {
    queue_push(&queue, id);
}
inline void IdQueue::push_back(Id id1, Id id2) {
    queue_push2(&queue, id1, id2);
}
inline Id IdQueue::operator[](int index) const {
    return queue.elements[index];
}
inline int * IdQueue::data() const noexcept {
    return queue.elements;
}
inline Queue & IdQueue::get_queue() noexcept {
    return queue;
}
inline int IdQueue::size() const noexcept {
    return queue.count;
}
inline void IdQueue::clear() noexcept {
    queue_empty(&queue);
}

inline void IdQueue::append(const IdQueue & src) {
    queue_insertn(&queue, size(), src.size(), src.data());
}

inline void IdQueue::reserve(int n) {
    queue_prealloc(&queue, n);
}

}  // namespace libdnf::rpm::solv

#endif  // LIBDNF_RPM_SOLV_ID_QUEUE_HPP
