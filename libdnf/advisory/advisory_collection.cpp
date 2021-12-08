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


#include "libdnf/advisory/advisory_collection.hpp"

#include "advisory/advisory_module_private.hpp"
#include "advisory/advisory_package_private.hpp"


namespace libdnf::advisory {

AdvisoryCollection::AdvisoryCollection(const libdnf::BaseWeakPtr & base, AdvisoryId advisory, int index)
    : base(base),
      advisory(advisory),
      index(index) {}

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
    auto & pool = get_pool(base);
    int count = 0;

    dataiterator_init(&di, *pool, 0, advisory.id, UPDATE_COLLECTIONLIST, 0, 0);
    while (dataiterator_step(&di)) {
        dataiterator_setpos(&di);
        if (count == index) {
            Dataiterator di_inner;
            dataiterator_init(&di_inner, *pool, 0, SOLVID_POS, UPDATE_COLLECTION, 0, 0);
            while (dataiterator_step(&di_inner)) {
                dataiterator_setpos(&di_inner);
                Id name = pool.lookup_id(SOLVID_POS, UPDATE_COLLECTION_NAME);
                Id evr = pool.lookup_id(SOLVID_POS, UPDATE_COLLECTION_EVR);
                Id arch = pool.lookup_id(SOLVID_POS, UPDATE_COLLECTION_ARCH);
                if (with_filemanes) {
                    filename = pool.lookup_str(SOLVID_POS, UPDATE_COLLECTION_FILENAME);
                }
                output.emplace_back(
                    AdvisoryPackage(new AdvisoryPackage::Impl(base, advisory, index, name, evr, arch, filename)));
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
    auto & pool = get_pool(base);
    int count = 0;

    dataiterator_init(&di, *pool, 0, advisory.id, UPDATE_COLLECTIONLIST, 0, 0);
    while (dataiterator_step(&di)) {
        dataiterator_setpos(&di);
        if (count == index) {
            Dataiterator di_inner;
            dataiterator_init(&di_inner, *pool, 0, SOLVID_POS, UPDATE_MODULE, 0, 0);
            while (dataiterator_step(&di_inner)) {
                dataiterator_setpos(&di_inner);
                Id name = pool.lookup_id(SOLVID_POS, UPDATE_MODULE_NAME);
                Id stream = pool.lookup_id(SOLVID_POS, UPDATE_MODULE_STREAM);
                Id version = pool.lookup_id(SOLVID_POS, UPDATE_MODULE_VERSION);
                Id context = pool.lookup_id(SOLVID_POS, UPDATE_MODULE_CONTEXT);
                Id arch = pool.lookup_id(SOLVID_POS, UPDATE_MODULE_ARCH);
                output.emplace_back(AdvisoryModule(
                    new AdvisoryModule::Impl(base, advisory, index, name, stream, version, context, arch)));
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
    return Advisory(base, advisory);
}

}  // namespace libdnf::advisory
