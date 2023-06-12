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

#ifndef LIBDNF5_ADVISORY_ADVISORY_REFERENCE_HPP
#define LIBDNF5_ADVISORY_ADVISORY_REFERENCE_HPP

#include "advisory.hpp"

#include <memory>
#include <vector>


namespace libdnf5::advisory {

class AdvisoryReference {
public:
    /// Get id of this advisory reference, this id is like a name of this reference
    /// (it is not libsolv id).
    ///
    /// @return id of this reference as std::string.
    std::string get_id() const;

    /// Get type of this reference.
    /// Possible reference types are: "bugzilla", "cve", "vendor".
    ///
    /// @return type of this reference as std::string.
    std::string get_type() const;

    /// Get type of this reference.
    ///
    /// @return Type of this reference as const char* !! (temporal value)
    const char * get_type_cstring() const;

    /// Get title of this reference.
    ///
    /// @return Title of this reference.
    std::string get_title() const;

    /// Get url of this reference.
    ///
    /// @return Url of this reference.
    std::string get_url() const;

private:
    friend class Advisory;

    /// Construct AdvisoryReference
    ///
    /// @param base     Reference to Base instance.
    /// @param advisory AdvisoryId into libsolv pool.
    /// @param index    Index of this reference in its advisory.
    /// @return New AdvisoryReference instance.
    AdvisoryReference(const BaseWeakPtr & base, AdvisoryId advisory, int index);

    BaseWeakPtr base;
    AdvisoryId advisory;

    /// We cannot store IDs of reference data (id, type, title, url) because they
    /// don't have ids set in libsolv (they are only strings), therefore we store
    /// index of the reference.
    int index;
};

}  // namespace libdnf5::advisory

#endif
