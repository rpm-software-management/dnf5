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

#ifndef LIBDNF5_CLI_ARGUMENT_PARSER_ERRORS_HPP
#define LIBDNF5_CLI_ARGUMENT_PARSER_ERRORS_HPP

#include "libdnf5-cli/exception.hpp"

#include <string>

namespace libdnf5::cli {

/// Parent for all ArgumentsParser runtime errors.
class ArgumentParserError : public Error {
    using Error::Error;
    const char * get_name() const noexcept override { return "ArgumentParserError"; }
};

/// Exception is thrown when conflicting arguments are used together.
class ArgumentParserConflictingArgumentsError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserConflictingArgumentsError"; }
};

/// Exception is thrown when no command is found.
class ArgumentParserMissingCommandError : public ArgumentParserError {
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserMissingCommandError"; }
};

/// Exception is thrown when a command requires a positional argument that was not found.
class ArgumentParserMissingPositionalArgumentError : public ArgumentParserError {
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserMissingPositionalArgumentError"; }
};

/// Exception is thrown when a positional argument has invalid format.
class ArgumentParserPositionalArgumentFormatError : public ArgumentParserError {
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserPositionalArgumentFormatError"; }
};

/// Exception is generated in the case of an unexpected argument.
class ArgumentParserUnknownArgumentError : public ArgumentParserError {
private:
    std::string command;
    std::string argument;

public:
    using ArgumentParserError::ArgumentParserError;

    template <typename... Ss>
    ArgumentParserUnknownArgumentError(
        const std::string command, const std::string argument, const BgettextMessage & format, Ss &&... args)
        : ArgumentParserError(format, std::forward<Ss>(args)...),
          command(command),
          argument(argument) {}
    const char * get_name() const noexcept override { return "ArgumentParserUnknownArgumentError"; }
    const std::string & get_command() const { return command; };
    const std::string & get_argument() const { return argument; };
};


/// Exception is thrown when an argument with the same ID is already registered.
class ArgumentParserGroupArgumentIdRegisteredError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserGroupArgumentIdRegisteredError"; }
};


/// Exception is thrown when the Argument `id` contains not allowed '.' character.
class ArgumentParserArgumentInvalidIdError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserArgumentInvalidIdError"; }
};


/// Exception is thrown when there are too few values for the positional argument.
class ArgumentParserPositionalArgFewValuesError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserPositionalArgFewValuesError"; }
};


/// Exception is thrown if the named argument requires a value and the value is missing.
class ArgumentParserNamedArgMissingValueError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserNamedArgMissingValueError"; }
};

/// Exception is thrown if the named argument is defined without a value and a value is present.
class ArgumentParserNamedArgValueNotExpectedError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserNamedArgValueNotExpectedError"; }
};


/// Exception is thrown when the command, named_argument, or positional argument was not found.
class ArgumentParserNotFoundError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserNotFoundError"; }
};

/// Exception is thrown when the command, named argument, positional argument, or group with the same ID is already registered.
class ArgumentParserIdAlreadyRegisteredError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserIdAlreadyRegisteredError"; }
};

/// Exception is thrown when the command, named argument, positiona argument, or group needs an additional argument.
class ArgumentParserMissingDependentArgumentError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserMissingDependentArgumentError"; }
};

/// Exception is thrown when the given argument value is not valid.
class ArgumentParserInvalidValueError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserInvalidValueError"; }
};

}  // namespace libdnf5::cli

#endif
