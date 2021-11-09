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

#include "pool.hpp"

extern "C" {
#include <solv/pool.h>
#include <solv/queue.h>
#include <solv/util.h>
}


namespace libdnf::solv {

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
    solv::get_repo(solvable).p_impl->solv_repo.internalize();
    return solvable_lookup_sourcepkg(solvable);
}

}  // namespace libdnf::solv
