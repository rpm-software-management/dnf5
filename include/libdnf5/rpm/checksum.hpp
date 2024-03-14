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

#ifndef LIBDNF5_RPM_CHECKSUM_HPP
#define LIBDNF5_RPM_CHECKSUM_HPP

#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/defs.h"

#include <memory>
#include <string>


namespace libdnf5::rpm {

/// Class contains checksum and checksum type
class LIBDNF_API Checksum {
public:
    ~Checksum();
    Checksum(const Checksum & src);
    Checksum & operator=(const Checksum & src);

    Checksum(Checksum && src) noexcept;
    Checksum & operator=(Checksum && src) noexcept;

    enum class Type { UNKNOWN, MD5, SHA1, SHA224, SHA256, SHA384, SHA512 };
    Type get_type() const noexcept;
    std::string get_type_str() const;

    /// Return checksum in hex format
    const std::string & get_checksum() const noexcept;


private:
    friend class Package;

    /// Require checksum in hex and libsolv checksum type
    LIBDNF_LOCAL Checksum(const char * checksum, int libsolv_type);

    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};


}  // namespace libdnf5::rpm

#endif  // LIBDNF5_RPM_CHECKSUM_HPP
