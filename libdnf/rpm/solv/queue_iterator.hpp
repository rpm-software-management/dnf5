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

#include <solv/queue.h>

#include <cstddef>
#include <iterator>

namespace libdnf::rpm::solv {

class IdQueueIterator {
public:
    explicit IdQueueIterator(const Queue * queue) noexcept;
    IdQueueIterator(const IdQueueIterator & other) noexcept = default;

    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Id;
    using pointer = void;
    using reference = Id;

    Id operator*() const noexcept { return queue->elements[current_index]; }

    IdQueueIterator & operator++() noexcept;
    IdQueueIterator operator++(int) noexcept;

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

inline IdQueueIterator::IdQueueIterator(const Queue * queue) noexcept : queue{queue} {
    begin();
}

inline IdQueueIterator & IdQueueIterator::operator++() noexcept {
    ++current_index;
    return *this;
}

inline IdQueueIterator IdQueueIterator::operator++(int) noexcept {
    IdQueueIterator ret(*this);
    ++*this;
    return ret;
}

}  // namespace libdnf::rpm::solv

#endif  // LIBDNF_RPM_SOLV_QUEUE_ITERATOR_HPP
