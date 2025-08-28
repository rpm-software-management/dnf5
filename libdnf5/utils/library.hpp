// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef _LIBDNF5_UTILS_LIBRARY_HPP
#define _LIBDNF5_UTILS_LIBRARY_HPP

#include "libdnf5/common/exception.hpp"

#include <string>


namespace libdnf5::utils {

/// Library exception
class LibraryError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::utils"; }
    const char * get_name() const noexcept override { return "LibraryError"; }
};

class LibraryLoadingError : public LibraryError {
public:
    using LibraryError::LibraryError;
    const char * get_name() const noexcept override { return "LibraryLoadingError"; }
};

class LibrarySymbolNotFoundError : public LibraryError {
public:
    using LibraryError::LibraryError;
    const char * get_name() const noexcept override { return "LibrarySymbolNotFoundError"; }
};


class Library {
public:
    explicit Library(const std::string & path);
    ~Library();

    Library(const Library &) = delete;
    Library(Library &&) = delete;
    Library & operator=(const Library &) = delete;
    Library & operator=(Library &&) = delete;

    const std::string & get_path() const noexcept { return path; }
    void * get_address(const char * symbol) const;

private:
    std::string path;
    void * handle;
};

}  // namespace libdnf5::utils

#endif
