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

#ifndef LIBDNF5_ADVISORY_ADVISORY_MODULE_HPP
#define LIBDNF5_ADVISORY_ADVISORY_MODULE_HPP

#include "advisory.hpp"

#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/defs.h"


namespace libdnf5::advisory {

class LIBDNF_API AdvisoryModule {
public:
    AdvisoryModule(const AdvisoryModule & src);
    AdvisoryModule(AdvisoryModule && src) noexcept;
    AdvisoryModule & operator=(const AdvisoryModule & src);
    AdvisoryModule & operator=(AdvisoryModule && src) noexcept;
    ~AdvisoryModule();

    /// Get name of this AdvisoryModule.
    ///
    /// @return Name of this AdvisoryModule as std::string.
    std::string get_name() const;

    /// Get stream of this AdvisoryModule.
    ///
    /// @return Stream of this AdvisoryModule as std::string.
    std::string get_stream() const;

    /// Get version of this AdvisoryModule.
    ///
    /// @return Version of this AdvisoryModule as std::string.
    std::string get_version() const;

    /// Get context of this AdvisoryModule.
    ///
    /// @return Context of this AdvisoryModule as std::string.
    std::string get_context() const;

    /// Get arch of this AdvisoryModule.
    ///
    /// @return Arch of this AdvisoryModule as std::string.
    std::string get_arch() const;

    /// Get NSVCA of this AdvisoryModule.
    ///
    /// @return NSVCA of this AdvisoryModule as std::string.
    std::string get_nsvca() const;

    /// Get AdvisoryId of Advisory this AdvisoryModule belongs to.
    ///
    /// @return AdvisoryId of this AdvisoryModule.
    AdvisoryId get_advisory_id() const;

    //TODO(amatej): we should be able to get Advisory and AdvisoryCollection information
    //              from a advisory module, returning new advisory object is one option but it might
    //              be better to set up so sort of inheritance among Advisory, AdvisoryCollection,
    //              AdvisoryPackage and AdvisoryModule.
    /// Get Advisory this AdvisoryModule belongs to.
    ///
    /// @return newly construted Advisory object of this AdvisoryModule.
    Advisory get_advisory() const;
    /// Get AdvisoryCollection this AdvisoryModule belongs to.
    ///
    /// @return newly construted AdvisoryCollection object of this AdvisoryModule.
    AdvisoryCollection get_advisory_collection() const;


private:
    friend class AdvisoryCollection;

    class LIBDNF_LOCAL Impl;
    LIBDNF_LOCAL AdvisoryModule(Impl * private_module);
    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5::advisory

#endif
