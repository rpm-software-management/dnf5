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

#ifndef LIBDNF5_ADVISORY_ADVISORY_COLLECTION_HPP
#define LIBDNF5_ADVISORY_ADVISORY_COLLECTION_HPP

#include "advisory.hpp"
#include "advisory_module.hpp"
#include "advisory_package.hpp"
#include "advisory_set.hpp"

#include "libdnf5/defs.h"

#include <vector>


namespace libdnf5::advisory {

//TODO(amatej): add unit tests for AdvisoryCollection
class LIBDNF_API AdvisoryCollection {
public:
    AdvisoryCollection(const AdvisoryCollection & src);
    AdvisoryCollection & operator=(const AdvisoryCollection & src);

    AdvisoryCollection(AdvisoryCollection && src) noexcept;
    AdvisoryCollection & operator=(AdvisoryCollection && src) noexcept;

    ~AdvisoryCollection();

    /// Whether this AdvisoryCollection is applicable. True when at least one AdvisoryModule in this
    /// AdvisoryCollection is active on the system, False otherwise.
    bool is_applicable() const;

    /// Get AdvisoryId of Advisory this AdvisoryCollection belongs to.
    ///
    /// @return AdvisoryId of this AdvisoryCollection.
    AdvisoryId get_advisory_id() const;

    /// Get all AdvisoryPackages stored in this AdvisoryCollection
    ///
    /// @return std::vector of AdvisorPackages used as output.
    std::vector<AdvisoryPackage> get_packages();

    /// Get all AdvisoryModules stored in this AdvisoryCollection
    ///
    /// @return std::vector of AdvisorModules.
    std::vector<AdvisoryModule> get_modules();

    //TODO(amatej): we should be able to get advisory information from a collection,
    //              returning new advisory object is one option but it might be better
    //              to set up so sort of inheritance among Advisory, AdvisoryCollection
    //              and possibly AdvisoryPackage, AdvisoryModule.
    /// Get Advisory this AdvisoryCollection belongs to.
    ///
    /// @return newly construted Advisory object of this AdvisoryCollection.
    Advisory get_advisory() const;

private:
    friend Advisory;
    friend AdvisoryModule;
    friend AdvisoryPackage;
    friend AdvisorySet;
    friend class AdvisoryQuery;

    LIBDNF_LOCAL AdvisoryCollection(const BaseWeakPtr & base, AdvisoryId advisory, int index);

    /// Get all AdvisoryPackages stored in this AdvisoryCollection
    ///
    /// @param output           std::vector of AdvisorPackages used as output.
    ///                         This is much faster than returning new std::vector and later joining
    ///                         them when collecting AdvisoryPackages from multiple collections.
    /// @param with_filenames   Filenames of AdvisoryPackages are not always useful, this allows skipping them.
    ///                         The filename is stored as a c string (not libsolv id) this incurs slowdown.
    LIBDNF_LOCAL void get_packages(std::vector<AdvisoryPackage> & output, bool with_filenames = false);

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::advisory

#endif
