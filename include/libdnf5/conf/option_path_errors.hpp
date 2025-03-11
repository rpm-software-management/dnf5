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

#ifndef LIBDNF5_CONF_OPTION_PATH_ERRORS_HPP
#define LIBDNF5_CONF_OPTION_PATH_ERRORS_HPP

#include "option_errors.hpp"

#include "libdnf5/defs.h"

namespace libdnf5 {

/// Exception that is generated when input path does not exist.
class LIBDNF_API OptionPathNotFoundError : public OptionValueNotAllowedError {
public:
    using OptionValueNotAllowedError::OptionValueNotAllowedError;
    const char * get_name() const noexcept override { return "OptionPathNotFoundError"; }
};

}  // namespace libdnf5

#endif
