// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
