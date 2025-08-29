// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
