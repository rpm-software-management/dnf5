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

#include "libdnf/rpm/checksum.hpp"

extern "C" {
#include <solv/chksum.h>
}

namespace libdnf::rpm {

std::string Checksum::get_type_str() const {
    auto type_str = solv_chksum_type2str(libsolv_type);
    return type_str ? type_str : "unknown";
}

Checksum::Type Checksum::get_type() const noexcept {
    switch (libsolv_type) {
        case REPOKEY_TYPE_MD5:
            return Type::MD5;
        case REPOKEY_TYPE_SHA1:
            return Type::SHA1;
        case REPOKEY_TYPE_SHA224:
            return Type::SHA224;
        case REPOKEY_TYPE_SHA256:
            return Type::SHA256;
        case REPOKEY_TYPE_SHA384:
            return Type::SHA384;
        case REPOKEY_TYPE_SHA512:
            return Type::SHA512;
        default:
            return Type::UNKNOWN;
    }
}

}  //  namespace libdnf::rpm
