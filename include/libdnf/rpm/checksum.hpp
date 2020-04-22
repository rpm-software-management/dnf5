/*
Copyright (C) 2018-2020 Red Hat, Inc.

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


#ifndef LIBDNF_RPM_CHECKSUM_HPP
#define LIBDNF_RPM_CHECKSUM_HPP

#include <string>

namespace libdnf::rpm {

// forward declarations
class Package;

/// Class contains checksum and checksum type
class Checksum {
public:
    enum class Type { UNKNOWN, MD5, SHA1, SHA224, SHA256, SHA384, SHA512 };
    Type get_type() const noexcept;
    std::string get_type_str() const;

    /// Return checksum in hex format
    const std::string & get_checksum() const noexcept { return checksum; };


private:
    friend class Package;

    /// Require checksum in hex and libsolv checksum type
    Checksum(const char * checksum, int libsolv_type);

    std::string checksum;
    int libsolv_type;
};


/// Constructor requires checksum in hex and libsolv checksum type
inline Checksum::Checksum(const char * checksum, int libsolv_type) : checksum(checksum), libsolv_type(libsolv_type) {}


}  // namespace libdnf::rpm

#endif  // LIBDNF_RPM_CHECKSUM_HPP
