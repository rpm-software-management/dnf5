/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef LIBDNF5_RPM_RELDEP_LIST_ITERATOR_HPP
#define LIBDNF5_RPM_RELDEP_LIST_ITERATOR_HPP

#include "reldep.hpp"

#include "libdnf5/defs.h"

#include <cstddef>
#include <iterator>
#include <memory>


namespace libdnf5::rpm {

class ReldepList;


class LIBDNF_API ReldepListIterator {
public:
    explicit ReldepListIterator(const ReldepList & reldep_list);
    ReldepListIterator(const ReldepListIterator & other);
    ~ReldepListIterator();

    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Reldep;
    using pointer = void;
    using reference = Reldep;

    Reldep operator*();

    ReldepListIterator & operator++();
    ReldepListIterator operator++(int);

    bool operator==(const ReldepListIterator & other) const;
    bool operator!=(const ReldepListIterator & other) const;

    void begin();
    void end();

private:
    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::rpm

#endif  // LIBDNF5_RPM_RELDEP_LIST_ITERATOR_HPP
