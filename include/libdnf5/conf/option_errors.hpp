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

#ifndef LIBDNF5_CONF_OPTION_ERRORS_HPP
#define LIBDNF5_CONF_OPTION_ERRORS_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"


namespace libdnf5 {

/// Option exception
class LIBDNF_API OptionError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5"; }
    const char * get_name() const noexcept override { return "OptionError"; }
};

/// Exception that is generated when an invalid input value is detected.
class LIBDNF_API OptionInvalidValueError : public OptionError {
public:
    using OptionError::OptionError;
    const char * get_name() const noexcept override { return "OptionInvalidValueError"; }
};

/// Exception that is generated when not allowed input value is detected.
class LIBDNF_API OptionValueNotAllowedError : public OptionInvalidValueError {
public:
    using OptionInvalidValueError::OptionInvalidValueError;
    const char * get_name() const noexcept override { return "OptionValueNotAllowedError"; }
};

/// Exception that is generated during read an empty Option.
class LIBDNF_API OptionValueNotSetError : public OptionError {
public:
    using OptionError::OptionError;
    const char * get_name() const noexcept override { return "OptionValueNotSetError"; }
};

}  // namespace libdnf5

#endif
