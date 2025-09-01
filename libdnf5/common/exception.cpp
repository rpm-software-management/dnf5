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

#include "libdnf5/common/exception.hpp"

#include "libdnf5/utils/bgettext/bgettext-lib.h"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <algorithm>
#include <charconv>
#include <system_error>


namespace libdnf5 {


AssertionError::AssertionError(const char * assertion, const SourceLocation & location, const std::string & message)
    : logic_error(message),
      condition(assertion),
      location(location) {}


const char * AssertionError::what() const noexcept {
    try {
        str_what = location.file_name + std::string(":") + std::to_string(location.source_line) + ": " +
                   location.function_name;
        if (condition) {
            str_what += std::string(": Assertion '") + condition + "' failed: ";
        } else {
            str_what += ": Assertion failed: ";
        }
        str_what += logic_error::what();
        return str_what.c_str();
    } catch (...) {
        return logic_error::what();
    }
}

UserAssertionError::UserAssertionError(
    const char * assertion, const SourceLocation & location, const std::string & message)
    : logic_error(message),
      condition(assertion),
      location(location) {}


const char * UserAssertionError::what() const noexcept {
    try {
        str_what = location.file_name + std::string(":") + std::to_string(location.source_line) + ": " +
                   location.function_name;
        if (condition) {
            str_what += std::string(": API Assertion '") + condition + "' failed: ";
        } else {
            str_what += ": API Assertion failed: ";
        }
        str_what += logic_error::what();
        return str_what.c_str();
    } catch (...) {
        return logic_error::what();
    }
}

SystemError::SystemError(int error_code) : Error(M_("System error")), error_code(error_code), has_user_message(false) {}

std::string SystemError::get_error_message() const {
    return std::system_category().default_error_condition(error_code).message();
}

static std::string format(const std::exception & e, std::size_t level, FormatDetailLevel detail) {
    std::string ret(level, ' ');
    if (const auto * ex = dynamic_cast<const Error *>(&e)) {
        switch (detail) {
            case FormatDetailLevel::WithDomainAndName: {
                ret += fmt::format("{}::{}: {}\n", ex->get_domain_name(), ex->get_name(), ex->what());
            } break;
            case FormatDetailLevel::WithName: {
                ret += fmt::format("{}: {}\n", ex->get_name(), ex->what());
            } break;
            case FormatDetailLevel::Plain: {
                ret += e.what();
                ret += '\n';
            } break;
        }
    } else {
        ret += e.what();
        ret += '\n';
    }
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception & e) {
        ret += format(e, level + 1, detail);
    } catch (...) {
    }

    return ret;
}

std::string format(const std::exception & e, FormatDetailLevel detail) {
    return format(e, 0, detail);
}

Error::Error(const Error & e) noexcept : std::runtime_error(e) {
    operator=(e);
}


Error & Error::operator=(const Error & e) noexcept {
    this->std::runtime_error::operator=(e);

    try {
        message = e.message;
        format = e.format;
        formatter = e.formatter;
    } catch (...) {
    }

    return *this;
}


const char * Error::what() const noexcept {
    if (!formatter) {
        // formatter not set means copy constructor or assignment operator failed
        return TM_(format, 1);
    }

    try {
        message = formatter(TM_(format, 1));
        return message.c_str();
    } catch (...) {
        return std::runtime_error::what();
    }
}

const char * SystemError::what() const noexcept {
    std::string error_message;
    try {
        error_message = std::system_category().default_error_condition(error_code).message();
    } catch (...) {
    }

    if (has_user_message) {
        try {
            message = fmt::format(
                "{}: ({}) - {}", formatter ? formatter(TM_(format, 1)) : TM_(format, 1), error_code, error_message);
        } catch (...) {
            return TM_(format, 1);
        }
    } else {
        try {
            message = fmt::format("({}) - {}", error_code, error_message);
        } catch (...) {
            if (error_message.empty()) {
                return std::runtime_error::what();
            }
            message = std::move(error_message);
        }
    }
    return message.c_str();
}

const char * FileSystemError::what() const noexcept {
    std::string error_message;
    try {
        error_message = std::system_category().default_error_condition(error_code).message();
    } catch (...) {
    }

    try {
        message = fmt::format(
            "{}: ({}) - {} [{}]",
            formatter ? formatter(TM_(format, 1)) : TM_(format, 1),
            error_code,
            error_message,
            std::string(path));
    } catch (...) {
        return TM_(format, 1);
    }

    return message.c_str();
}

const char * RuntimeError::get_description() const noexcept {
    return _("General RuntimeError exception");
}

}  // namespace libdnf5
