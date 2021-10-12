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

#ifndef LIBDNF_UTILS_EXCEPTION_HPP
#define LIBDNF_UTILS_EXCEPTION_HPP

#include <fmt/format.h>

#include <stdexcept>

#define LIBDNF_LOCATION { __FILE__, __LINE__, __PRETTY_FUNCTION__ }

/// An assert macro that throws `libdnf::AssertionError`.
///
/// @param format The format string for the message.
/// @param ... The format arguments.
/// @exception libdnf::AssertionError Thrown always.
#define libdnf_throw_assertion(msg_format, ...) \
    (throw libdnf::AssertionError(              \
        nullptr, LIBDNF_LOCATION, fmt::format(msg_format, ##__VA_ARGS__)))

/// An assert macro that throws `libdnf::AssertionError` when `condition` is not met.
///
/// @param condition The assertion condition. Throws AssertionError if it's not met.
/// @param format The format string for the message.
/// @param ... The format arguments.
/// @exception libdnf::AssertionError Thrown when condition is not met.
#define libdnf_assert(condition, msg_format, ...) \
    (static_cast<bool>(condition)                 \
         ? void(0)                                \
         : throw libdnf::AssertionError(          \
               #condition, LIBDNF_LOCATION, fmt::format(msg_format, ##__VA_ARGS__)))

/// Indicates the availability of `libdnf_assert` and` libdnf_throw_assertion` macros.
/// These macros may be removed in the future. E.g. when migrating the asserts implementation
/// to C++20 `std::source_location`.
#define LIBDNF_ASSERTION_MACROS 1

namespace libdnf {

/// The source_location structure represents location information in the source code.
/// Specifically, the file name, line number, and function name.
struct SourceLocation {
    const char * file_name;
    unsigned int source_line;
    const char * function_name;
};

/// An AssertionError is a fault in the program logic, it is thrown when an
/// incorrect sequence of actions has led to an invalid state in which it is
/// impossible to continue running the program.
class AssertionError : public std::logic_error {
public:
    AssertionError(
        const char * assertion,
        const SourceLocation & location,
        const std::string & message);

    const char * what() const noexcept override;
    const char * assertion() const noexcept { return condition; }
    const char * file_name() const noexcept { return location.file_name; }
    unsigned int source_line() const noexcept { return location.source_line; }
    const char * function_name() const noexcept { return location.function_name; }
    const char * message() const noexcept { return logic_error::what(); }

private:
    const char * condition;
    SourceLocation location;
    mutable std::string str_what;
};

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

    /// Returns explanatory message. All informations formatted to string.
    const char * what() const noexcept override;

protected:
    mutable std::string str_what;
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
    SystemError(int error_code, const std::string & what);
    SystemError(int error_code, const char * what);
    const char * get_domain_name() const noexcept override { return "SystemError"; }
    const char * get_name() const noexcept override { return name; }
    const char * get_description() const noexcept override;

    /// Returns original error code
    int get_error_code() const noexcept { return error_code; }

private:
    static constexpr const char * NAME_PREFIX = "Errno_";
    static constexpr std::size_t NAME_PREFIX_LEN = std::char_traits<char>::length(NAME_PREFIX);
    int error_code;
    char name[NAME_PREFIX_LEN + 10];
    mutable std::string description;
};

class InvalidPointer : public Exception {
public:
    using Exception::Exception;
    const char * get_name() const noexcept override { return "InvalidPointer"; }
    const char * get_description() const noexcept override { return "Invalid pointer"; }
};

/// Formats the explanatory string of an exception.
/// If the exception is nested, recurses to format the explanatory of the exception it holds.
std::string format(const std::exception & e, std::size_t level = 0);

}  // namespace libdnf

#endif
