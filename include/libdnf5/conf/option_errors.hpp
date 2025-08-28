// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
