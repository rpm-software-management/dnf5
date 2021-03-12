/*
Copyright (C) 2020-2021 Red Hat, Inc.

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
    SystemError(int error_code, const std::string & what) : RuntimeError(what), error_code{error_code} {}
    SystemError(int error_code, const char * what) : RuntimeError(what), error_code{error_code} {}
    SystemError(int error_code, const std::string & what, const std::string & path) : RuntimeError(what), error_code{error_code}, path{path} {}
    const char * get_domain_name() const noexcept override { return "system"; }
    const char * get_name() const noexcept override { return "SystemError"; }
    const char * get_description() const noexcept override { return "System error"; }
    int get_error_code() const noexcept { return error_code; }
    const std::string & get_path() const noexcept { return path; }
    const char * what() const noexcept override {
        try {
            // example: "General RuntimeError exception: [Errno 1] "
            str_what = get_description();
            str_what += ": [Errno ";
            str_what += std::to_string(get_error_code());
            str_what += "] ";

            // example: "Operation not permitted"
            str_what += std::system_category().default_error_condition(error_code).message();

            // example: ": Unable to install package"
            if (runtime_error::what()) {
                str_what += ": ";
                str_what += runtime_error::what();
            }

            // example: ": /path/to/file"
            if (!path.empty()) {
                str_what += ": ";
                str_what += get_path();
            }

            return str_what.c_str();
        } catch (...) {
            return Exception::what();
        }
    }

private:
    const int error_code;
    const std::string path;
};


}  // namespace libdnf

#endif
