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

#include "library.hpp"

#include "utils/bgettext/bgettext-lib.h"

#include <dlfcn.h>
#include <libdnf/utils/format.hpp>

namespace microdnf::utils {

Library::Library(const std::string & path) : path(path) {
    handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        const char * err_msg = dlerror();  // returns localized mesage, problem with later translation
        throw std::runtime_error(
            libdnf::utils::sformat(_("Cannot load shared library \"{}\": {}"), path, std::string(err_msg)));
    }
}

Library::~Library() {
    dlclose(handle);
}

void * Library::get_address(const char * symbol) const {
    dlerror();  // Clear any existing error
    void * address = dlsym(handle, symbol);
    if (!address) {
        const char * err_msg = dlerror();  // returns localized mesage, problem with later translation
        if (err_msg) {
            throw std::runtime_error(libdnf::utils::sformat(
                _("Cannot obtain address of symbol \"{}\": {}"), std::string(symbol), std::string(err_msg)));
        }
    }
    return address;
}

}  // namespace microdnf::utils
