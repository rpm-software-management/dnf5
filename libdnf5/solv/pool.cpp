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

#include "pool.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <fnmatch.h>

extern "C" {
#include <solv/pool.h>
#include <solv/queue.h>
#include <solv/util.h>
}


namespace libdnf5::solv {

// Check if an illegal vendor change occurs when an installed solvable is replaced by a new solvable.
// Returns 1 if the vendor change is illegal, otherwise returns 0.
int RpmPool::callback_policy_illegal_vendorchange(::Pool * libsolv_pool, Solvable * installed, Solvable * new_solv) {
    auto & vendor_change_manager = reinterpret_cast<RpmPool *>(libsolv_pool->appdata)->vendor_change_manager;
    return vendor_change_manager.is_vendor_change_allowed(*installed, *new_solv) ? 0 : 1;
}


TempEvr::TempEvr(const Pool & pool, const char * evr) {
    split_evr = pool_alloctmpspace(*pool, static_cast<int>(strlen(evr)) + 1);
    strcpy(split_evr, evr);

    for (e = split_evr + 1; *e != ':' && *e != '-' && *e != '\0'; ++e) {
        ;
    }

    if (*e == '-') {
        *e = '\0';
        v = split_evr;
        r = e + 1;
        e = nullptr;
    } else if (*e == '\0') {
        v = split_evr;
        e = nullptr;
        r = nullptr;
    } else {  // *e == ':'
        *e = '\0';
        v = e + 1;
        e = split_evr;

        r = v + 1;
        while (*r != '-' && *r != '\0') {
            ++r;
        }

        if (*r != '\0') {
            *r = '\0';
            ++r;
        }
    }
}


TempEvr::~TempEvr() {
    // TODO(lukash) replace the tmpspace allocation with something more reliable
    // right now we don't free the space, as the `const char *`s are returned
    // from Pool::get_{epoch,version,release} and they'd get deallocated
    // immediately
    //pool_freetmpspace(*pool, split_evr);
}


Pool::~Pool() {
    pool_free(pool);
}


/// Returns a temporary object allocated by pool_alloctmpspace
const char * pool_solvable_epoch_optional_2str(
    const Pool * pool, ::Pool * libsolv_pool, Id id, bool with_epoch) noexcept {
    const char * e;
    const char * name = pool->get_name(id);
    const char * evr = pool->get_evr(id);
    const char * arch = pool->get_arch(id);
    bool present_epoch = false;

    for (e = evr + 1; *e != '-' && *e != '\0'; ++e) {
        if (*e == ':') {
            present_epoch = true;
            break;
        }
    }
    char * output_string;
    int evr_length, arch_length;
    int extra_epoch_length = 0;
    int name_length = static_cast<int>(strlen(name));
    evr_length = static_cast<int>(strlen(evr));
    arch_length = static_cast<int>(strlen(arch));
    if (!present_epoch && with_epoch) {
        extra_epoch_length = 2;
    } else if (present_epoch && !with_epoch) {
        extra_epoch_length = static_cast<int>(evr - e - 1);
    }

    output_string = pool_alloctmpspace(libsolv_pool, name_length + evr_length + extra_epoch_length + arch_length + 3);

    strcpy(output_string, name);

    if (evr_length || extra_epoch_length > 0) {
        output_string[name_length++] = '-';

        if (extra_epoch_length > 0) {
            output_string[name_length++] = '0';
            output_string[name_length++] = ':';
            output_string[name_length] = '\0';
        }
    }

    if (evr_length) {
        if (extra_epoch_length >= 0) {
            strcpy(output_string + name_length, evr);
        } else {
            strcpy(output_string + name_length, evr - extra_epoch_length);
            evr_length = evr_length + extra_epoch_length;
        }
    }

    if (arch_length) {
        output_string[name_length + evr_length] = '.';
        strcpy(output_string + name_length + evr_length + 1, arch);
    }
    return output_string;
}

const char * Pool::get_nevra_without_epoch(Id id) const noexcept {
    return pool_solvable_epoch_optional_2str(this, pool, id, false);
}

const char * Pool::get_nevra_with_epoch(Id id) const noexcept {
    return pool_solvable_epoch_optional_2str(this, pool, id, true);
}

const char * Pool::get_str_from_pool(Id keyname, Id advisory, int index) const {
    Dataiterator di;
    const char * str = NULL;
    int count = 0;

    dataiterator_init(&di, pool, 0, advisory, UPDATE_REFERENCE, 0, 0);
    while (dataiterator_step(&di)) {
        dataiterator_setpos(&di);
        if (count++ == index) {
            str = lookup_str(SOLVID_POS, keyname);
            break;
        }
    }
    dataiterator_free(&di);

    return str;
}


unsigned long Pool::get_epoch_num(Id id) const {
    const auto evr = split_evr(get_evr(id));
    if (evr.e) {
        char * endptr;
        unsigned long converted = std::strtoul(evr.e, &endptr, 10);

        if (converted == ULONG_MAX || *endptr != '\0') {
            // TODO(lukash) throw proper exception class
            throw RuntimeError(M_("Failed to convert epoch \"{}\" to number"), std::string(evr.e));
        }

        return converted;
    }
    return 0;
}


std::string Pool::get_full_nevra(Id id) const {
    Solvable * solvable = id2solvable(id);
    const char * name = id2str(solvable->name);
    const char * evr = id2str(solvable->evr);
    const char * arch = id2str(solvable->arch);

    bool add_zero_epoch = true;
    if (*evr != '\0') {
        for (const char * e = evr + 1; *e != '-' && *e != '\0'; ++e) {
            if (*e == ':') {
                add_zero_epoch = false;
                break;
            }
        }
    }

    std::string res;
    // potentially wasting up to 4 bytes (2 for the zero epoch and 2 for a '-' and a '.')
    res.reserve(strlen(name) + strlen(evr) + strlen(arch) + 4);

    res.append(name);

    if (*evr != '\0') {
        res.append("-");

        if (add_zero_epoch) {
            res.append("0:");
        }

        res.append(evr);
    }

    if (*arch != '\0') {
        res.append(".");
        res.append(arch);
    }

    return res;
}


const char * Pool::get_sourcerpm(Id id) const {
    Solvable * solvable = id2solvable(id);
    solv::get_repo(solvable).internalize();
    return solvable_lookup_sourcepkg(solvable);
}

std::pair<std::string, std::string> CompsPool::split_solvable_name(std::string_view solvable_name) {
    auto delimiter_position = solvable_name.find(":");
    if (delimiter_position == std::string::npos) {
        return std::pair<std::string, std::string>("", "");
    }
    return std::pair<std::string, std::string>(
        solvable_name.substr(0, delimiter_position), solvable_name.substr(delimiter_position + 1, std::string::npos));
}

}  // namespace libdnf5::solv
