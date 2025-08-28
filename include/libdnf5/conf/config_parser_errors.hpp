// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_CONF_CONFIG_PARSER_ERRORS_HPP
#define LIBDNF5_CONF_CONFIG_PARSER_ERRORS_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"


namespace libdnf5 {

/// Error accessing config file other than ENOENT; e.g. we don't have read permission
class LIBDNF_API InaccessibleConfigError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5"; }
    const char * get_name() const noexcept override { return "InaccessibleConfigError"; }
};

/// Configuration file is missing
class LIBDNF_API MissingConfigError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5"; }
    const char * get_name() const noexcept override { return "MissingConfigError"; }
};

/// Configuration file is invalid
class LIBDNF_API InvalidConfigError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5"; }
    const char * get_name() const noexcept override { return "InvalidConfigError"; }
};

class LIBDNF_API ConfigParserError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5"; }
    const char * get_name() const noexcept override { return "ConfigParserError"; }
};

class LIBDNF_API ConfigParserSectionNotFoundError : public ConfigParserError {
public:
    explicit ConfigParserSectionNotFoundError(const std::string & section);
    const char * get_name() const noexcept override { return "ConfigParserSectionNotFoundError"; }
};

class LIBDNF_API ConfigParserOptionNotFoundError : public ConfigParserError {
public:
    explicit ConfigParserOptionNotFoundError(const std::string & section, const std::string & option);
    const char * get_name() const noexcept override { return "ConfigParserOptionNotFoundError"; }
};

}  // namespace libdnf5

#endif
