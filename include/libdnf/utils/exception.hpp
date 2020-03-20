/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_UTILS_EXCEPTION_HPP
#define LIBDNF_UTILS_EXCEPTION_HPP


#include <stdexcept>
#include <system_error>

namespace libdnf {

/// Base class of libdnf exceptions. Each exception define at least domain_name, name, and description.
/// These information can be used to serialize exception into a string.
class Exception : public std::runtime_error {
public:
    using runtime_error::runtime_error;

    /// Returns domain name (namespace) of the exception as text.
    virtual const char * get_domain_name() const noexcept { return "libdnf"; }

    /// Returns exception class name as text.
    virtual const char * get_name() const noexcept { return "Exception"; }

    /// Returns exception class description.
    virtual const char * get_description() const noexcept { return "General libdnf exception"; }

    /// Returns the original unformatted message passed to the exception.
    const char * get_message() const noexcept { return runtime_error::what(); }

    /// Returns "clever" message. Description and messages in one string.
    const char * what() const noexcept override {
        try {
            str_what = std::string(get_description()) + ": " + runtime_error::what();
            return str_what.c_str();
        } catch (...) {
            return runtime_error::what();
        }
    }

protected:
    mutable std::string str_what;
};

class LogicError : public Exception {
public:
    using Exception::Exception;
    const char * get_name() const noexcept override { return "LogicError"; }
    const char * get_description() const noexcept override { return "General LogicError exception"; }
};

class RuntimeError : public Exception {
public:
    using Exception::Exception;
    const char * get_name() const noexcept override { return "RuntimeError"; }
    const char * get_description() const noexcept override { return "General RuntimeError exception"; }
};

/// Wraps the error code originated from the operating system.
class SystemError : public RuntimeError {
public:
    SystemError(int code, const std::string & what) : RuntimeError(what), error_code(code) {}
    SystemError(int code, const char * what) : RuntimeError(what), error_code(code) {}
    const char * get_domain_name() const noexcept override { return "system"; }
    const char * get_name() const noexcept override { return "SystemError"; }
    const char * get_description() const noexcept override { return "System error"; }
    const char * what() const noexcept override {
        try {
            str_what = get_description();
            str_what +=
                ": " + std::system_category().default_error_condition(error_code).message() + ": " + get_message();
            return str_what.c_str();
        } catch (...) {
            return Exception::what();
        }
    }
    int get_code() const noexcept { return error_code; }

private:
    int error_code;
};

}  // namespace libdnf

#endif
