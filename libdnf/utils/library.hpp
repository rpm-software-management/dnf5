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

#ifndef _LIBDNF_UTILS_LIBRARY_HPP
#define _LIBDNF_UTILS_LIBRARY_HPP

#include "libdnf/common/exception.hpp"

#include <string>


namespace libdnf::utils {

class Library {
public:
    /// Library exception
    class Exception : public RuntimeError {
    public:
        using RuntimeError::RuntimeError;
        const char * get_domain_name() const noexcept override { return "libdnf::Library"; }
        const char * get_name() const noexcept override { return "Exception"; }
        const char * get_description() const noexcept override { return "Library exception"; }
    };

    class CantLoad : public Exception {
    public:
        using Exception::Exception;
        const char * get_name() const noexcept override { return "CantLoad"; }
        const char * get_description() const noexcept override { return "Can't load shared library"; }
    };

    class NotFound : public Exception {
    public:
        using Exception::Exception;
        const char * get_name() const noexcept override { return "NotFound"; }
        const char * get_description() const noexcept override { return "Can't obtain address of symbol"; }
    };

    explicit Library(const std::string & path);
    ~Library();

    Library(const Library &) = delete;
    Library(Library &&) = delete;
    Library & operator =(const Library &) = delete;
    Library & operator =(Library &&) = delete;

    const std::string & get_path() const noexcept { return path; }
    void * get_address(const char * symbol) const;

private:
    std::string path;
    void * handle;
};

}  // namespace libdnf::utils

#endif
