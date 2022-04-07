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

#ifndef LIBDNF_MODULE_MODULE_ERRORS_HPP
#define LIBDNF_MODULE_MODULE_ERRORS_HPP

#include "libdnf/common/exception.hpp"


namespace libdnf::module {


class ModuleError : public Error {
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf::module"; }
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


}  // namespace libdnf::module


#endif  // LIBDNF_MODULE_MODULE_ERRORS_HPP
