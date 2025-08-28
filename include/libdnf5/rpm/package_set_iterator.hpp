// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_RPM_PACKAGE_SET_ITERATOR_HPP
#define LIBDNF5_RPM_PACKAGE_SET_ITERATOR_HPP

#include "package.hpp"

#include "libdnf5/defs.h"

#include <cstddef>
#include <iterator>
#include <memory>


namespace libdnf5::rpm {

class PackageSet;


class LIBDNF_API PackageSetIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Package;
    using pointer = void;
    using reference = Package;

    PackageSetIterator(const PackageSetIterator & other);
    ~PackageSetIterator();

    PackageSetIterator & operator=(const PackageSetIterator & other);

    static PackageSetIterator begin(const PackageSet & package_set);
    static PackageSetIterator end(const PackageSet & package_set);

    Package operator*();

    PackageSetIterator & operator++();
    PackageSetIterator operator++(int);

    bool operator==(const PackageSetIterator & other) const;
    bool operator!=(const PackageSetIterator & other) const;

    void begin();
    void end();

private:
    explicit PackageSetIterator(const PackageSet & package_set);

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::rpm

#endif  // LIBDNF5_RPM_PACKAGE_SET_ITERATOR_HPP
