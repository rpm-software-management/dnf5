// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


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
