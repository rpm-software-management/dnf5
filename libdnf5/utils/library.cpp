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

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <dlfcn.h>

namespace libdnf5::utils {

Library::Library(const std::string & path) : path(path) {
    // the NODELETE is needed to not garbage plugin's libraries global data,
    // like when the plugin uses glib, then it could break its type system
    handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_NODELETE);
    if (!handle) {
        const char * err_msg = dlerror();  // returns localized message, problem with later translation
        throw LibraryLoadingError(M_("Cannot load shared library \"{}\": {}"), path, std::string(err_msg));
    }
}

Library::~Library() {
    dlclose(handle);
}

void * Library::get_address(const char * symbol) const {
    dlerror();  // Clear any existing error
    void * address = dlsym(handle, symbol);
    if (!address) {
        const char * err_msg = dlerror();  // returns localized message, problem with later translation
        throw LibrarySymbolNotFoundError(
            M_("Cannot obtain address of symbol \"{}\": {}"),
            std::string(symbol),
            err_msg ? std::string(err_msg) : "unknown error");
    }
    return address;
}

}  // namespace libdnf5::utils
