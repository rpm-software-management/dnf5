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

#ifndef LIBDNF5_CONF_OPTION_BINDS_ERRORS_HPP
#define LIBDNF5_CONF_OPTION_BINDS_ERRORS_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"

#include <string>


namespace libdnf5 {

class LIBDNF_API OptionBindsError : public Error {
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5"; }
    const char * get_name() const noexcept override { return "OptionBindsError"; }
};

class LIBDNF_API OptionBindsOptionNotFoundError : public OptionBindsError {
public:
    explicit OptionBindsOptionNotFoundError(const std::string & id);
    const char * get_name() const noexcept override { return "OptionBindsOptionNotFoundError"; }
};

class LIBDNF_API OptionBindsOptionAlreadyExistsError : public OptionBindsError {
public:
    explicit OptionBindsOptionAlreadyExistsError(const std::string & id);
    const char * get_name() const noexcept override { return "OptionBindsOptionAlreadyExistsError"; }
};

}  // namespace libdnf5

#endif
