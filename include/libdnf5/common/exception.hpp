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

#ifndef LIBDNF5_COMMON_EXCEPTION_HPP
#define LIBDNF5_COMMON_EXCEPTION_HPP

#include "libdnf5/defs.h"
#include "libdnf5/utils/bgettext/bgettext-mark-common.h"
#include "libdnf5/utils/format.hpp"

#include <fmt/format.h>

#include <filesystem>
#include <functional>
#include <stdexcept>
#include <type_traits>

#define LIBDNF_LOCATION {__FILE__, __LINE__, __PRETTY_FUNCTION__}

/// An assert macro that throws `libdnf5::AssertionError`.
///
/// @param msg_format The format string for the message.
/// @param ... The format arguments.
/// @exception libdnf5::AssertionError Thrown always.
#define libdnf_throw_assertion(msg_format, ...) \
    (throw libdnf5::AssertionError(nullptr, LIBDNF_LOCATION, fmt::format(msg_format, ##__VA_ARGS__)))

/// An assert macro that throws `libdnf5::AssertionError` when `condition` is not met.
///
/// @param condition The assertion condition. Throws AssertionError if it's not met.
/// @param msg_format The format string for the message.
/// @param ... The format arguments.
/// @exception libdnf5::AssertionError Thrown when condition is not met.
#define libdnf_assert(condition, msg_format, ...) \
    (static_cast<bool>(condition)                 \
         ? void(0)                                \
         : throw libdnf5::AssertionError(#condition, LIBDNF_LOCATION, fmt::format(msg_format, ##__VA_ARGS__)))

/// An assert macro that throws `libdnf5::UserAssertionError` when `condition` is not met.
///
/// @param condition The assertion condition. Throws UserAssertionError if it's not met.
/// @param msg_format The format string for the message.
/// @param ... The format arguments.
/// @exception libdnf5::UserAssertionError Thrown when condition is not met.
#define libdnf_user_assert(condition, msg_format, ...) \
    (static_cast<bool>(condition)                      \
         ? void(0)                                     \
         : throw libdnf5::UserAssertionError(#condition, LIBDNF_LOCATION, fmt::format(msg_format, ##__VA_ARGS__)))

/// Indicates the availability of `libdnf_assert` and` libdnf_throw_assertion` macros.
/// These macros may be removed in the future. E.g. when migrating the asserts implementation
/// to C++20 `std::source_location`.
#define LIBDNF_ASSERTION_MACROS 1

namespace libdnf5 {

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
class LIBDNF_API AssertionError : public std::logic_error {
public:
    explicit AssertionError(const char * assertion, const SourceLocation & location, const std::string & message);

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

/// A UserAssertionError is an error which is thrown when the
/// libdnf public API is used in an unexpected way and continuing
/// would led to an invalid state.

/// For the bindings users, this exception is intended to be translated
/// into a standard runtime exception which could be handled,
/// whereas with the previous `AssertionError` exception the process
/// is terminated and the system state is captured for debugging purposes.
class LIBDNF_API UserAssertionError : public std::logic_error {
public:
    explicit UserAssertionError(const char * assertion, const SourceLocation & location, const std::string & message);

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


// The exception stores the passed arguments and uses them in the catch phase.
// A problem can occur if the stored argument points to a value in memory that it does not own.
// The value that the stored argument points to may no longer be in memory - it is destroyed
// when the program leaves the scope of the variable with the value.
// A safer solution than passing a pointer is to use passing by value. For example, pass
// "std::string" instead of `char *` or `std::string_view`.
/// "Concept" defines safe types that can be passed by value as arguments to the `libdnf5::Error`
/// and `libdnf5::SystemError` exception constructors.
/// Pointers to void and nullptr are allowed because they are not dereferenced, but their value is printed.
template <typename T>
concept AllowedErrorArgTypes =
    std::is_arithmetic_v<T> || std::is_base_of_v<std::string, T> || std::is_null_pointer_v<T> ||
    std::is_same_v<const void *, T> || std::is_same_v<void *, T>;


/// Base class for libdnf exceptions. Virtual methods `get_name()` and
/// `get_domain_name()` should always return the exception's class name and its
/// namespace (including enclosing class names in case the exception is nested in
/// other classes) respectively.
class LIBDNF_API Error : public std::runtime_error {
public:
    /// A constructor that supports formatting the error message.
    ///
    /// @param format The format string for the message.
    /// @param args The format arguments.
    template <AllowedErrorArgTypes... Args>
    explicit Error(BgettextMessage format, Args... args)
        : std::runtime_error(b_gettextmsg_get_id(format)),
          format(format),
          // stores the format args in the lambda's closure
          formatter([args...](const char * format) { return libdnf5::utils::sformat(format, args...); }) {}

    Error(const Error & e) noexcept;
    Error & operator=(const Error & e) noexcept;

    const char * what() const noexcept override;

    /// @return The exception class name.
    virtual const char * get_name() const noexcept { return "Error"; }

    /// @return The domain name (namespace and enclosing class names) of the exception.
    virtual const char * get_domain_name() const noexcept { return "libdnf5"; }

protected:
    mutable std::string message;
    BgettextMessage format;
    std::function<std::string(const char * format)> formatter;
};


/// An exception class for system errors represented by the `errno` error code.
class LIBDNF_API SystemError : public Error {
public:
    /// Constructs the error from the `errno` error code and generates the
    /// message from the system error description.
    ///
    /// @param error_code The `errno` of the error.
    explicit SystemError(int error_code);

    /// Constructs the error from the `errno` error code and a formatted message.
    /// The formatted message is prepended to the generated system error message.
    ///
    /// @param error_code The `errno` of the error.
    /// @param format The format string for the message.
    /// @param args The format arguments.
    template <AllowedErrorArgTypes... Args>
    explicit SystemError(int error_code, BgettextMessage format, Args... args)
        : Error(format, std::forward<Args>(args)...),
          error_code(error_code),
          has_user_message(true) {}

    const char * what() const noexcept override;

    const char * get_domain_name() const noexcept override { return "libdnf5"; }
    const char * get_name() const noexcept override { return "SystemError"; }

    /// @return The error code (`errno`) of the error.
    int get_error_code() const noexcept { return error_code; }

    /// @return The system error message associated with the error code.
    std::string get_error_message() const;

private:
    int error_code;
    bool has_user_message;
};

/// An exception class for file system errors represented by the `errno` error code and a path.
class LIBDNF_API FileSystemError : public Error {
public:
    /// Constructs the error from the `errno` error code, filesystem path and a formatted message.
    /// The formatted message is prepended to the generated system error message.
    ///
    /// @param error_code The `errno` of the error.
    /// @param filesystem::path The `path` to the file.
    /// @param format The format string for the message.
    /// @param args The format arguments.
    template <AllowedErrorArgTypes... Args>
    explicit FileSystemError(int error_code, std::filesystem::path path, BgettextMessage format, Args... args)
        : Error(format, std::forward<Args>(args)...),
          error_code(error_code),
          path(std::move(path)) {}

    const char * what() const noexcept override;

    const char * get_domain_name() const noexcept override { return "libdnf5::utils::fs"; }
    const char * get_name() const noexcept override { return "FileSystemError"; }

    /// @return The error code (`errno`) of the error.
    int get_error_code() const noexcept { return error_code; }

private:
    int error_code;
    std::filesystem::path path;
};


// TODO(lukash) This class is used throughout the code where more specific exceptions should be thrown.
// Kept as a reminder to replace all those with the correct exception classes.
class LIBDNF_API RuntimeError : public Error {
public:
    using Error::Error;
    const char * get_name() const noexcept override { return "RuntimeError"; }
    virtual const char * get_description() const noexcept;
};


/// A template of the `NestedException` class that is thrown by the `throw_with_nested` function.
template <typename TException>
class NestedException : public TException, public std::nested_exception {
public:
    explicit NestedException(const TException & ex) : TException(ex) {}
    explicit NestedException(TException && ex) : TException(static_cast<TException &&>(ex)) {}
};

/// Throws an exception that also stores the currently active exception.
///
/// It does the same thing as `std::throw_with_nested(TException && ex)`, except that instead of
/// an **unspecified** type that is publicly derived from both `std::nested_exception` and
/// `std::decay<TException>::type`, it throws a type **`NestedException<std::decay<TException>::type>`**
/// that is publicly derived from both `std::nested_exception` and `std::decay<TException>::type`.
///
/// In other words, it replaces the unspecified type (the type defined by the implementation in the standard library)
/// with our specification. Knowing the type can simplify exception handling in some cases. For example, avoiding
/// the need to define another type for SWIG.
template <typename TException>
[[noreturn]] inline void throw_with_nested(TException && ex) {
    using TUpEx = typename std::decay<TException>::type;

    using IsCopyConstructible = std::conjunction<std::is_copy_constructible<TUpEx>, std::is_move_constructible<TUpEx>>;
    static_assert(IsCopyConstructible::value, "throw_with_nested argument must be CopyConstructible");

    if constexpr (!std::is_final_v<TUpEx> && std::is_class_v<TUpEx> && !std::is_base_of_v<std::nested_exception, TUpEx>)
        throw NestedException<TUpEx>{std::forward<TException>(ex)};
    throw std::forward<TException>(ex);
}


/// Describes the contents of the error message generated by the `format` function.
/// It is only considered for exceptions inheriting from `libdnf5::Error`. Others always use the `Plain` format.
enum class FormatDetailLevel {
    /// "what()\n"
    Plain,
    /// "get_name(): what()\n"
    WithName,
    /// "get_domain_name()::get_name(): what()\n"
    WithDomainAndName,
};

/// Formats the error message of an exception.
/// If the exception is nested, recurses to format the message of the exception it holds.
LIBDNF_API std::string format(const std::exception & e, FormatDetailLevel detail);

}  // namespace libdnf5

#endif
