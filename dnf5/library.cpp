// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "library.hpp"

#include <dlfcn.h>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/format.hpp>

namespace dnf5::utils {

Library::Library(const std::string & path) : path(path) {
    handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        const char * err_msg = dlerror();  // returns localized message, problem with later translation
        throw std::runtime_error(
            libdnf5::utils::sformat(_("Cannot load shared library \"{}\": {}"), path, std::string(err_msg)));
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
        throw std::runtime_error(libdnf5::utils::sformat(
            _("Cannot obtain address of symbol \"{}\": {}"),
            std::string(symbol),
            err_msg ? std::string(err_msg) : "unknown error"));
    }
    return address;
}

}  // namespace dnf5::utils
