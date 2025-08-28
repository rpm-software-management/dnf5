// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_ADVISORY_ADVISORY_HPP
#define LIBDNF5_ADVISORY_ADVISORY_HPP

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/defs.h"

#include <string>
#include <vector>


namespace libdnf5::advisory {

class AdvisoryCollection;
class AdvisoryReference;

struct AdvisoryId {
public:
    AdvisoryId() = default;
    explicit AdvisoryId(int id) : id(id) {}

    bool operator==(const AdvisoryId & other) const noexcept { return id == other.id; };
    bool operator!=(const AdvisoryId & other) const noexcept { return id != other.id; };

    int id{0};
};

/// An advisory, represents advisory used to track security updates
class LIBDNF_API Advisory {
public:
    Advisory(const Advisory & src);
    Advisory & operator=(const Advisory & src);

    Advisory(Advisory && src) noexcept;
    Advisory & operator=(Advisory && src) noexcept;

    bool operator==(const Advisory & other) const noexcept;

    bool operator!=(const Advisory & other) const noexcept;

    /// Destroy the Advisory object
    ~Advisory();

    /// Get name of this advisory.
    ///
    /// @return Name of this advisory as std::string.
    std::string get_name() const;

    /// Get severity of this advisory.
    ///
    /// @return Severity of this advisory as std::string.
    std::string get_severity() const;

    /// Get type of this advisory.
    /// Possible types are: "security", "bugfix", "enhancement", "newpackage".
    ///
    /// @return type of this advisory as std::string.
    std::string get_type() const;

    /// Get buildtime of this advisory. Libsolv combines issued and updated dates
    /// into buildtime by always using the newer one.
    ///
    /// @return buildtime of this advisory.
    unsigned long long get_buildtime() const;

    /// Get vendor of this advisory.
    ///
    /// @return Vendor of this advisory as std::string.
    std::string get_vendor() const;

    /// Get description of this advisory.
    ///
    /// @return Description of this advisory as std::string.
    std::string get_description() const;

    /// Get title of this advisory.
    ///
    /// @return Title of this advisory as std::string.
    std::string get_title() const;

    /// Get status of this advisory.
    ///
    /// @return Status of this advisory as std::string.
    std::string get_status() const;

    /// Get rights of this advisory.
    ///
    /// @return Rights of this advisory as std::string.
    std::string get_rights() const;

    /// Get message of this advisory.
    ///
    /// @return Message of this advisory as std::string.
    std::string get_message() const;

    /// Get AdvisoryId.
    ///
    /// @return AdvisoryId of this advisory.
    AdvisoryId get_id() const;

    /// Get all references of specified type from this advisory.
    /// Possible refenrece types are: "bugzilla", "cve", "vendor".
    ///
    /// @param types     What types of references to get. If not specified gets all types.
    /// @return Vector of AdvisoryReference objects.
    std::vector<AdvisoryReference> get_references(std::vector<std::string> types = {}) const;

    /// Get all collections from this advisory.
    ///
    /// @return Vector of AdvisoryCollection objects.
    std::vector<AdvisoryCollection> get_collections() const;

    /// Check whether at least one collection from this advisory is applicable.
    ///
    /// @return True if applicable, False otherwise.
    bool is_applicable() const;

protected:
    /// Construct the Advisory object
    ///
    /// @param base   WeakPtr to libdnf5::base::Base instance which this object belongs to.
    /// @param id     AdvisoryId into libsolv pool.
    /// @return New Advisory instance.
    Advisory(const BaseWeakPtr & base, AdvisoryId id);

private:
    friend class AdvisoryCollection;
    friend class AdvisoryModule;
    friend class AdvisoryPackage;
    friend class AdvisorySetIterator;
    friend class AdvisorySet;

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::advisory

#endif
