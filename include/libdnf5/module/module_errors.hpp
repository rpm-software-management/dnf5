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

#ifndef LIBDNF5_MODULE_MODULE_ERRORS_HPP
#define LIBDNF5_MODULE_MODULE_ERRORS_HPP

#include "libdnf5/common/exception.hpp"


namespace libdnf5::module {


class ModuleError : public Error {
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::module"; }
    const char * get_name() const noexcept override { return "ModuleError"; }
};


class NoModuleError : public ModuleError {
public:
    using ModuleError::ModuleError;
    const char * get_name() const noexcept override { return "NoModuleError"; }
};


class NoStreamError : public ModuleError {
public:
    using ModuleError::ModuleError;
    const char * get_name() const noexcept override { return "NoStreamError"; }
};


class EnabledStreamError : public ModuleError {
public:
    using ModuleError::ModuleError;
    const char * get_name() const noexcept override { return "EnabledStreamError"; }
};


class EnableMultipleStreamsError : public ModuleError {
public:
    using ModuleError::ModuleError;
    const char * get_name() const noexcept override { return "EnableMultipleStreamsError"; }
};


class ModuleConflictError : public ModuleError {
public:
    using ModuleError::ModuleError;
    const char * get_name() const noexcept override { return "ModuleConflictError"; }
};


class ModuleResolveError : public ModuleError {
public:
    using ModuleError::ModuleError;
    const char * get_name() const noexcept override { return "ModuleResolveError"; }
};


}  // namespace libdnf5::module


#endif  // LIBDNF5_MODULE_MODULE_ERRORS_HPP
