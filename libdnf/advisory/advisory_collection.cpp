/*
Copyright (C) 2021 Red Hat, Inc.

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


#include "libdnf/advisory/advisory_collection.hpp"

#include "libdnf/advisory/advisory_module_private.hpp"
#include "libdnf/advisory/advisory_package_private.hpp"
#include "libdnf/rpm/solv_sack_impl.hpp"

namespace libdnf::advisory {

AdvisoryCollection::AdvisoryCollection(const libdnf::rpm::SolvSackWeakPtr & sack, AdvisoryId advisory, int index)
    : sack(sack)
    , advisory(advisory)
    , index(index) {}

bool AdvisoryCollection::is_applicable() const {
    //TODO(amatej): check if collection is applicable
    return true;
}

std::vector<AdvisoryPackage> AdvisoryCollection::get_packages() {
    std::vector<AdvisoryPackage> output;
    get_packages(output, true);
    return output;
}

std::vector<AdvisoryModule> AdvisoryCollection::get_modules() {
    std::vector<AdvisoryModule> output;
    get_modules(output);
    return output;
}

void AdvisoryCollection::get_packages(std::vector<AdvisoryPackage> & output, bool with_filemanes) {
    Dataiterator di;
    const char * filename = nullptr;
    Pool * pool = sack->p_impl->get_pool();
    int count = 0;

    dataiterator_init(&di, pool, 0, advisory.id, UPDATE_COLLECTIONLIST, 0, 0);
    while (dataiterator_step(&di)) {
        dataiterator_setpos(&di);
        if (count == index) {
            Dataiterator di_inner;
            dataiterator_init(&di_inner, pool, 0, SOLVID_POS, UPDATE_COLLECTION, 0, 0);
            while (dataiterator_step(&di_inner)) {
                dataiterator_setpos(&di_inner);
                Id name = pool_lookup_id(pool, SOLVID_POS, UPDATE_COLLECTION_NAME);
                Id evr = pool_lookup_id(pool, SOLVID_POS, UPDATE_COLLECTION_EVR);
                Id arch = pool_lookup_id(pool, SOLVID_POS, UPDATE_COLLECTION_ARCH);
                if (with_filemanes) {
                    filename = pool_lookup_str(pool, SOLVID_POS, UPDATE_COLLECTION_FILENAME);
                }
                output.emplace_back(
                    AdvisoryPackage(new AdvisoryPackage::Impl(*sack, advisory, index, name, evr, arch, filename)));
            }
            dataiterator_free(&di_inner);
            break;
        }
        count++;
    }
    dataiterator_free(&di);
}

void AdvisoryCollection::get_modules(std::vector<AdvisoryModule> & output) {
    Dataiterator di;
    Pool * pool = sack->p_impl->get_pool();
    int count = 0;

    dataiterator_init(&di, pool, 0, advisory.id, UPDATE_COLLECTIONLIST, 0, 0);
    while (dataiterator_step(&di)) {
        dataiterator_setpos(&di);
        if (count == index) {
            Dataiterator di_inner;
            dataiterator_init(&di_inner, pool, 0, SOLVID_POS, UPDATE_MODULE, 0, 0);
            while (dataiterator_step(&di_inner)) {
                dataiterator_setpos(&di_inner);
                Id name = pool_lookup_id(pool, SOLVID_POS, UPDATE_MODULE_NAME);
                Id stream = pool_lookup_id(pool, SOLVID_POS, UPDATE_MODULE_STREAM);
                Id version = pool_lookup_id(pool, SOLVID_POS, UPDATE_MODULE_VERSION);
                Id context = pool_lookup_id(pool, SOLVID_POS, UPDATE_MODULE_CONTEXT);
                Id arch = pool_lookup_id(pool, SOLVID_POS, UPDATE_MODULE_ARCH);
                output.emplace_back(AdvisoryModule(
                    new AdvisoryModule::Impl(*sack, advisory, index, name, stream, version, context, arch)));
            }
            dataiterator_free(&di_inner);
            break;
        }
        count++;
    }
    dataiterator_free(&di);
}

AdvisoryId AdvisoryCollection::get_advisory_id() const {
    return advisory;
}

Advisory AdvisoryCollection::get_advisory() const {
    return Advisory(sack, advisory);
}

}  // namespace libdnf::advisory
