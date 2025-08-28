// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
