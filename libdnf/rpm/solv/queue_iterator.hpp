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


#ifndef LIBDNF_RPM_SOLV_QUEUE_ITERATOR_HPP
#define LIBDNF_RPM_SOLV_QUEUE_ITERATOR_HPP

#include "libdnf/rpm/solv_sack.hpp"

#include <bits/stdc++.h>
#include <solv/pooltypes.h>
#include <solv/queue.h>

#include <iterator>


namespace libdnf::rpm::solv {


class IdQueue;


class IdQueueIterator {
public:
    explicit IdQueueIterator(const Queue * queue);
    IdQueueIterator(const IdQueueIterator & other) = default;

    using iterator_category = std::forward_iterator_tag;
    using difference_type = Id;
    using value_type = Id;
    using pointer = void;
    using reference = void;

    Id operator*() const { return current_value; }

    IdQueueIterator & operator++();
    IdQueueIterator operator++(int) { return ++(*this); }

    bool operator==(const IdQueueIterator & other) const { return current_index == other.current_index; }
    bool operator!=(const IdQueueIterator & other) const { return current_index != other.current_index; }

    void begin();
    void end();

protected:
    const Queue * get_queue() const noexcept { return queue; }

private:
    constexpr static int END = -2;

    // pointer to a queue owned by IdQueue
    const Queue * queue;

    int current_index;
    Id current_value;
};


inline IdQueueIterator::IdQueueIterator(const Queue * queue) : queue{queue} {
    current_index = -1;
    ++(*this);
}

inline void IdQueueIterator::begin() {
    current_index = -1;
    ++(*this);
}

inline void IdQueueIterator::end() {
    current_value = END;
    current_index = queue->count;
}

inline IdQueueIterator & IdQueueIterator::operator++() {
    current_index++;
    if (current_index >= queue->count) {
        current_value = END;
    } else {
        current_value = queue->elements[current_index];
    }
    return *this;
}


}  // namespace libdnf::rpm::solv


#endif  // LIBDNF_RPM_SOLV_QUEUE_ITERATOR_HPP
