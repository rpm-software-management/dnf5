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

#ifndef LIBDNF_COMMON_EXCEPTION_HPP
#define LIBDNF_COMMON_EXCEPTION_HPP

#include <fmt/format.h>

#include <stdexcept>
#include <functional>

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


/// Base class for libdnf exceptions. Virtual methods `get_name()` and
/// `get_domain_name()` should always return the exception's class name and its
/// namespace (including enclosing class names in case the exception is nested in
/// other classes) respectively.
class Error : public std::runtime_error {
public:
    /// A constructor that supports formatting the error message.
    ///
    /// @param format The format string for the message.
    /// @param args The format arguments.
    template<typename... Ss>
    Error(const std::string & format, Ss&&... args)
        : std::runtime_error(format),
        // stores the format args in the lambda's closure
        formatter([args...](const char * format) {
            return fmt::format(format, args...);
        }) {}

    Error(const Error & e) noexcept;
    Error & operator=(const Error & e) noexcept;

    const char * what() const noexcept override;

    /// @return The exception class name.
    virtual const char * get_name() const noexcept { return "Error"; }

    /// @return The domain name (namespace and enclosing class names) of the exception.
    virtual const char * get_domain_name() const noexcept { return "libdnf"; }
protected:
    mutable std::string message;
    std::function<std::string(const char * format)> formatter;
};


/// An exception class for system errors represented by the `errno` error code.
class SystemError : public Error {
public:
    /// Constructs the error from the `errno` error code and generates the
    /// message from the system error description.
    ///
    /// @param error_code The `errno` of the error.
    SystemError(int error_code) : Error("System error"), error_code(error_code), has_user_message(false) {}

    /// Constructs the error from the `errno` error code and a formatted message.
    /// The formatted message is prepended to the generated system error message.
    ///
    /// @param error_code The `errno` of the error.
    /// @param format The format string for the message.
    /// @param args The format arguments.
    template <typename... Ss>
    SystemError(int error_code, const std::string & format, Ss &&... args)
        : Error(format, std::forward<Ss>(args)...),
          error_code(error_code),
          has_user_message(true) {}

    const char * what() const noexcept override;

    const char * get_domain_name() const noexcept override { return "libdnf"; }
    const char * get_name() const noexcept override { return "SystemError"; }

    /// @return The error code (`errno`) of the error.
    int get_error_code() const noexcept { return error_code; }

    /// @return The system error message associated with the error code.
    std::string get_error_message() const;

private:
    int error_code;
    bool has_user_message;
};


// TODO(lukash) This class is used throughout the code where more specific exceptions should be thrown.
// Kept as a reminder to replace all those with the correct exception classes.
class RuntimeError : public Error {
public:
    using Error::Error;
    const char * get_name() const noexcept override { return "RuntimeError"; }
    virtual const char * get_description() const noexcept;
};


/// Formats the error message of an exception.
/// If the exception is nested, recurses to format the message of the exception it holds.
std::string format(const std::exception & e, bool with_domain);

}  // namespace libdnf

#endif
