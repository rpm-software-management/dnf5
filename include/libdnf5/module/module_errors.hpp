// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
