// Copyright Contributors to the DNF5 project.
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


#ifndef LIBDNF5_ADVISORY_ADVISORY_SET_ITERATOR_HPP
#define LIBDNF5_ADVISORY_ADVISORY_SET_ITERATOR_HPP

#include "advisory.hpp"

#include "libdnf5/defs.h"

#include <cstddef>
#include <iterator>
#include <memory>


namespace libdnf5::advisory {

class AdvisorySet;


class LIBDNF_API AdvisorySetIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Advisory;
    using pointer = void;
    using reference = Advisory;

    AdvisorySetIterator(const AdvisorySetIterator & other);
    ~AdvisorySetIterator();

    AdvisorySetIterator & operator=(const AdvisorySetIterator & other);

    static AdvisorySetIterator begin(const AdvisorySet & advisory_set);
    static AdvisorySetIterator end(const AdvisorySet & advisory_set);

    Advisory operator*();

    AdvisorySetIterator & operator++();
    AdvisorySetIterator operator++(int);

    bool operator==(const AdvisorySetIterator & other) const;
    bool operator!=(const AdvisorySetIterator & other) const;

    void begin();
    void end();

private:
    LIBDNF_LOCAL explicit AdvisorySetIterator(const AdvisorySet & advisory_set);

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::advisory

#endif  // LIBDNF5_ADVISORY_ADVISORY_SET_ITERATOR_HPP
