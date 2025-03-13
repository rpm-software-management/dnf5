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
