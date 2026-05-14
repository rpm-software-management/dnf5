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

#include "libdnf5/rpm/checksum.hpp"

#include "libdnf5/common/exception.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

extern "C" {
#include <solv/chksum.h>
}

namespace libdnf5::rpm {

class Checksum::Impl {
public:
    Impl(const char * checksum, int libsolv_type) : libsolv_type(libsolv_type) {
        if (checksum != nullptr) {
            this->checksum = checksum;
        }
    }

private:
    friend Checksum;

    std::string checksum;
    int libsolv_type;
};

/// Constructor requires checksum in hex and libsolv checksum type
Checksum::Checksum(const char * checksum, int libsolv_type) : p_impl(new Impl(checksum, libsolv_type)) {}

Checksum::~Checksum() = default;

Checksum::Checksum(const Checksum & src) = default;
Checksum::Checksum(Checksum && src) noexcept = default;

Checksum & Checksum::operator=(const Checksum & src) = default;
Checksum & Checksum::operator=(Checksum && src) noexcept = default;

std::string Checksum::get_type_str() const {
    auto type_str = solv_chksum_type2str(p_impl->libsolv_type);
    return type_str ? type_str : "unknown";
}


Checksum::Type Checksum::get_type() const noexcept {
    switch (p_impl->libsolv_type) {
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


int Checksum::checksum_type_to_libsolv(Checksum::Type type) {
    switch (type) {
        case Checksum::Type::MD5:
            return REPOKEY_TYPE_MD5;
        case Checksum::Type::SHA1:
            return REPOKEY_TYPE_SHA1;
        case Checksum::Type::SHA224:
            return REPOKEY_TYPE_SHA224;
        case Checksum::Type::SHA256:
            return REPOKEY_TYPE_SHA256;
        case Checksum::Type::SHA384:
            return REPOKEY_TYPE_SHA384;
        case Checksum::Type::SHA512:
            return REPOKEY_TYPE_SHA512;
        default:
            throw libdnf5::RuntimeError(M_("Unknown checksum type"));
    }
}


const std::string & Checksum::get_checksum() const noexcept {
    return p_impl->checksum;
};

}  // namespace libdnf5::rpm
