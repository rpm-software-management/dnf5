/*
Copyright (C) 2018-2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "package_private.hpp"


namespace libdnf::rpm::solv {

const char * get_full_nevra(Pool * pool, libdnf::rpm::PackageId package_id) {
    Solvable * solvable = get_solvable(pool, package_id);
    const char * name = pool_id2str(pool, solvable->name);
    const char * evr = pool_id2str(pool, solvable->evr);
    const char * arch = pool_id2str(pool, solvable->arch);
    bool present_epoch = false;

    for (const char * e = evr + 1; *e != '-' && *e != '\0'; ++e) {
        if (*e == ':') {
            present_epoch = true;
            break;
        }
    }
    char * output_string;
    int extra_epoch_length = 0;
    int name_length = static_cast<int>(strlen(name));
    int evr_length = static_cast<int>(strlen(evr));
    int arch_length = static_cast<int>(strlen(arch));
    if (!present_epoch) {
        extra_epoch_length = 2;
    }

    output_string = pool_alloctmpspace(
        pool, name_length + evr_length + extra_epoch_length + arch_length + 3);

    strcpy(output_string, name);

    output_string[name_length++] = '-';

    if (extra_epoch_length == 2) {
        output_string[name_length++] = '0';
        output_string[name_length++] = ':';
        output_string[name_length] = '\0';
    }

    if (evr_length) {
        strcpy(output_string + name_length, evr);
        name_length += evr_length;
    }

    if (arch_length) {
        output_string[name_length++] = '.';
        strcpy(output_string + name_length, arch);
    }
    return output_string;
}

}  // namespace libdnf::rpm::solv

