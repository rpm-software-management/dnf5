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


#ifndef LIBDNF_RPM_PACKAGE_HPP
#define LIBDNF_RPM_PACKAGE_HPP

#include "checksum.hpp"
#include "reldep_list.hpp"

#include "libdnf/repo/repo_query.hpp"
#include "libdnf/transaction/transaction_item_reason.hpp"

#include <string>
#include <vector>


namespace libdnf {

class Goal;

}  // namespace libdnf


namespace libdnf::base {

class Transaction;

}  // namespace libdnf::base


namespace libdnf::rpm {

struct PackageId {
public:
    PackageId() = default;
    explicit PackageId(int id) : id(id) {}

    bool operator==(const PackageId & other) const noexcept { return id == other.id; };
    bool operator!=(const PackageId & other) const noexcept { return id != other.id; };
    bool operator<(const PackageId & other) const noexcept { return id < other.id; };


    int id{0};
};

// IMPORTANT: Package methods MUST NOT be 'noexcept'
// because accessing deleted sack throws an exception.

// @replaces libdnf:libdnf/hy-package.h:struct:DnfPackage
// @replaces dnf:dnf/package.py:class:Package
class Package {
public:
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_identical(DnfPackage * pkg)
    bool operator==(const Package & other) const noexcept;
    bool operator!=(const Package & other) const noexcept;
    bool operator<(const Package & other) const noexcept { return id < other.id; }

    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_id(DnfPackage * pkg)
    PackageId get_id() const noexcept { return id; };

    /// @return RPM package Name (`RPMTAG_NAME`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.name
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_name(DnfPackage *pkg);
    std::string get_name() const;

    /// @return RPM package Epoch (`RPMTAG_EPOCH`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.e
    // @replaces dnf:dnf/package.py:attribute:Package.epoch
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_epoch(DnfPackage *pkg);
    std::string get_epoch() const;

    /// @return RPM package Version (`RPMTAG_VERSION`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.v
    // @replaces dnf:dnf/package.py:attribute:Package.version
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_version(DnfPackage *pkg);
    std::string get_version() const;

    /// @return RPM package Release (`RPMTAG_RELEASE`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.r
    // @replaces dnf:dnf/package.py:attribute:Package.release
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_release(DnfPackage *pkg);
    std::string get_release() const;

    /// @return RPM package Arch (`RPMTAG_ARCH`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.a
    // @replaces dnf:dnf/package.py:attribute:Package.arch
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_arch(DnfPackage * pkg)
    std::string get_arch() const;

    /// @return RPM package EVR (Epoch:Version-Release). If the Epoch is 0, it is omitted from the output.
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.evr
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_evr(DnfPackage * pkg)
    std::string get_evr() const;

    /// @return RPM package NEVRA (Name-Epoch:Version-Release.Arch). If the Epoch is 0, it is omitted from the output.
    /// @since 5.0
    //
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_nevra(DnfPackage * pkg)
    std::string get_nevra() const;

    /// @return RPM package NEVRA (Name-Epoch:Version-Release.Arch). The Epoch is always present even if it is 0.
    /// @since 5.0
    //
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_nevra(DnfPackage * pkg)
    std::string get_full_nevra() const;

    /// @return RPM package Group (`RPMTAG_GROUP`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.group
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_group(DnfPackage * pkg)
    std::string get_group() const;

    /// @return File size of the RPM package.
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.downloadsize
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_downloadsize(DnfPackage * pkg)
    unsigned long long get_package_size() const;

    /// @return Size the RPM package should occupy after installing on disk (`RPMTAG_LONGSIZE`).
    ///         The information is always present - it is retrieved from rpmdb if the package is installed or from repodata if the package is available.
    /// @since 5.0
    /// @note The actual size on disk may vary based on block size and filesystem overhead.
    ///       Libdnf doesn't provide any method to compute the actual size on disk.
    //
    // @replaces dnf:dnf/package.py:attribute:Package.installsize
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_installsize(DnfPackage * pkg)
    unsigned long long get_install_size() const;

    /// @return RPM package License (`RPMTAG_LICENSE`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.license
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_license(DnfPackage * pkg)
    std::string get_license() const;

    /// @return RPM package source package filename (`RPMTAG_SOURCERPM`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.sourcerpm
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_sourcerpm(DnfPackage * pkg)
    std::string get_sourcerpm() const;

    /// @return RPM package build timestamp (`RPMTAG_BUILDTIME`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.buildtime
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_buildtime(DnfPackage * pkg)
    unsigned long long get_build_time() const;

    // TODO not supported by libsolv: https://github.com/openSUSE/libsolv/issues/400
    //std::string get_build_host();

    /// @return RPM package Packager (`RPMTAG_PACKAGER`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.packager
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_packager(DnfPackage * pkg)
    std::string get_packager() const;

    /// @return RPM package Vendor (`RPMTAG_VENDOR`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.vendor
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_vendor(DnfPackage * pkg)
    std::string get_vendor() const;

    /// @return RPM package URL (project home address) (`RPMTAG_URL`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.url
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_url(DnfPackage * pkg)
    std::string get_url() const;

    /// @return RPM package Summary (`RPMTAG_SUMMARY`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.summary
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_summary(DnfPackage * pkg)
    std::string get_summary() const;

    /// @return RPM package Description (`RPMTAG_DESCRIPTION`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.description
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_description(DnfPackage * pkg)
    std::string get_description() const;

    // DEPENDENCIES

    /// @return List of RPM package Provides (`RPMTAG_PROVIDENAME`, `RPMTAG_PROVIDEFLAGS`, `RPMTAG_PROVIDEVERSION`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.provides
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_provides(DnfPackage * pkg)
    ReldepList get_provides() const;

    /// @return List of RPM package Requires (`RPMTAG_REQUIRENAME`, `RPMTAG_REQUIREFLAGS`, `RPMTAG_REQUIREVERSION`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.requires
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_requires(DnfPackage * pkg)
    ReldepList get_requires() const;

    /// @return List of RPM package Requires(pre).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.requires_pre
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_requires_pre(DnfPackage * pkg)
    ReldepList get_requires_pre() const;

    /// @return List of RPM package Conflicts (`RPMTAG_CONFLICTNAME`, `RPMTAG_CONFLICTFLAGS`, `RPMTAG_CONFLICTVERSION`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.conflicts
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_conflicts(DnfPackage * pkg)
    ReldepList get_conflicts() const;

    /// @return List of RPM package Obsoletes (`RPMTAG_OBSOLETENAME`, `RPMTAG_OBSOLETEFLAGS`, `RPMTAG_OBSOLETEVERSION`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.obsoletes
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_obsoletes(DnfPackage * pkg)
    ReldepList get_obsoletes() const;

    // @replaces dnf:dnf/package.py:attribute:Package.prereq_ignoreinst
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_prereq_ignoreinst(DnfPackage * pkg)
    // TODO(dmach): docstring, consider renaming to get_requires_pre_ignoreinst()
    ReldepList get_prereq_ignoreinst() const;

    // @replaces dnf:dnf/package.py:attribute:Package.regular_requires
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_regular_requires(DnfPackage * pkg)
    // TODO(dmach): docstring, consider renaming to get_requires_regular()
    ReldepList get_regular_requires() const;

    // WEAK DEPENDENCIES

    /// @return List of RPM package Recommends (`RPMTAG_RECOMMENDNAME`, `RPMTAG_RECOMMENDFLAGS`, `RPMTAG_RECOMMENDVERSION`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.recommends
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_recommends(DnfPackage * pkg)
    ReldepList get_recommends() const;

    /// @return List of RPM package Suggests (`RPMTAG_SUGGESTNAME`, `RPMTAG_SUGGESTFLAGS`, `RPMTAG_SUGGESTVERSION`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.suggests
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_suggests(DnfPackage * pkg)
    ReldepList get_suggests() const;

    /// @return List of RPM package Enhances (`RPMTAG_ENHANCENAME`, `RPMTAG_ENHANCEFLAGS`, `RPMTAG_ENHANCEVERSION`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.enhances
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_enhances(DnfPackage * pkg)
    ReldepList get_enhances() const;

    /// @return List of RPM package Supplements (`RPMTAG_SUPPLEMENTNAME`, `RPMTAG_SUPPLEMENTFLAGS`, `RPMTAG_SUPPLEMENTVERSION`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.supplements
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_supplements(DnfPackage * pkg)
    ReldepList get_supplements() const;

    // ===== FILE LIST (filelists.xml) =====

    /// @return List of files and directories the RPM package contains (`RPMTAG_FILENAMES`).
    ///         If file lists are not loaded, empty list is returned.
    /// @since 5.0
    /// @note Information whether the returned files are actual files, directories or ghosted files is not available.
    //
    // @replaces dnf:dnf/package.py:attribute:Package.files
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_files(DnfPackage * pkg)
    std::vector<std::string> get_files() const;

    // ===== CHANGELOGS (other.xml) =====

    // TODO get_changelogs - requires changelog
    // @replaces dnf:dnf/package.py:attribute:Package.changelogs
    // @replaces libdnf:libdnf/hy-package-private.hpp:function:dnf_package_get_changelogs(DnfPackage * pkg)
    // void get_changelogs() const;

    // ===== REPODATA =====

    /// @return RPM package baseurl from repodata (`<location xml:base="...">`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.baseurl
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_baseurl(DnfPackage * pkg)
    std::string get_baseurl() const;

    /// @return RPM package relative path/location from repodata (`<location href="...">`).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.location
    // @replaces dnf:dnf/package.py:attribute:Package.relativepath
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_location(DnfPackage * pkg)
    std::string get_location() const;

    /// @return Checksum object representing RPM package checksum and its type (`<checksum type="type">checksum</checksum>`).
    /// @since 5.0
    //
    // TODO(dmach): add overridden method that allows specifying checksum type?
    // TODO(dmach): repodata can contain multiple checksums - how are we going to handle these?
    // @replaces dnf:dnf/package.py:attribute:Package.chksum
    // @replaces libdnf:libdnf/hy-package-private.hpp:function:dnf_package_get_chksum(DnfPackage *pkg, int *type)
    Checksum get_checksum() const;

    /// @return Checksum object representing RPM package header checksum and its type.
    /// @since 5.0
    //
    // TODO(dmach): add overridden method that allows specifying checksum type?
    // TODO(dmach): repodata can contain multiple checksums - how are we going to handle these?
    // TODO(dmach): does it come only from rpmdb or file? (I don't think it is part of repodata)
    // @replaces dnf:dnf/package.py:attribute:Package.hdr_chksum
    // @replaces libdnf:libdnf/hy-package-private.hpp:function:dnf_package_get_hdr_chksum(DnfPackage *pkg, int *type)
    Checksum get_hdr_checksum() const;

    /// @return RPM package header end (the header is located between file position 0 and the returned offset).
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.hdr_end
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_hdr_end(DnfPackage * pkg)
    unsigned long long get_hdr_end() const;

    /// @return Number of media (usually a CD/DVD disc) on which the package is located.
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.medianr
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_medianr(DnfPackage * pkg)
    unsigned long long get_media_number() const;

    // ===== SYSTEM =====

    /// @return Path to the RPM package on the local file system:
    ///         * If the package is a local package, return its path
    ///         * If the package is a remote package downloaded to cache, return path to the cache
    ///         * If the package is a remote package that hasn't been downloaded yet, return path to the cache
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.localPkg
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_local_baseurl(DnfPackage * pkg)
    std::string get_package_path() const;

    /// @return `true` if the package is installed on the system, `false` otherwise.
    /// @since 5.0
    //
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_installed(DnfPackage * pkg)
    // @replaces dnf:dnf/package.py:attribute:Package.installed
    bool is_installed() const;

    /// TODO is_local
    // @replaces dnf:dnf/package.py:method:Package.localPkg(self)
    // @replaces libdnf:libdnf/dnf-package.h:function:dnf_package_is_local(DnfPackage * pkg)
    //bool is_local() const;

    /// TODO get_from_repo_id - requires swdb
    /// For an installed package, return id of repo from the package was installed.
    /// For an available package, return an empty string.
    //
    // @replaces dnf:dnf/package.py:attribute:Package.ui_from_repo
    // @replaces libdnf:libdnf/dnf-package.h:function:dnf_package_get_origin(DnfPackage * pkg)
    // void get_from_repo_id() const;

    /// @return The unix timestamp (seconds since epoch) of when the package was installed.
    ///         Return 0 if the package is not installed.
    //
    // @replaces dnf:dnf/package.py:attribute:Package.installtime
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_installtime(DnfPackage * pkg)
    unsigned long long get_install_time() const;

    /// @return The rpmdb database id (primary key) of the installed RPM package.
    /// @since 5.0
    //
    // @replaces dnf:dnf/package.py:attribute:Package.rpmdbid
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_rpmdbid(DnfPackage * pkg)
    unsigned long long get_rpmdbid() const;

    /// @return A `Repo` object that represents the repository the package belongs to.
    /// @since 5.0
    /// @note This isn't the repository the package was installed from.
    //
    // @replaces dnf:dnf/package.py:attribute:Package.repo
    // @replaces libdnf:libdnf/dnf-package.h:function:dnf_package_get_repo(DnfPackage * pkg)
    libdnf::repo::RepoWeakPtr get_repo() const;

    /// @return Id of the repository the package belongs to.
    /// @since 5.0
    /// @note This isn't the repository the package was installed from.
    //
    // @replaces dnf:dnf/package.py:attribute:Package.repoid
    // @replaces dnf:dnf/package.py:attribute:Package.reponame
    // @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_reponame(DnfPackage * pkg)
    std::string get_repo_id() const;

    // TODO(dmach): getBugUrl() not possible due to lack of support in libsolv and metadata?

    /// @return Resolved reason why a package was installed.
    ///         A package can be installed due to multiple reasons, only the most significant is returned.
    /// @since 5.0
    //
    // TODO(dmach): return actual value from data in PackageSack
    // TODO(dmach): throw an exception when getting a reason for an available package (it should work only for installed)
    libdnf::transaction::TransactionItemReason get_reason() const {
        return libdnf::transaction::TransactionItemReason::UNKNOWN;
    }

protected:
    // @replaces libdnf:libdnf/dnf-package.h:function:dnf_package_new(DnfSack *sack, Id id)
    Package(const PackageSackWeakPtr & sack, PackageId id);

    /// @return RPM package Name as a `const char *`.
    /// @since 5.0
    /// @warning The value points to the underlying sack and shares its life-cycle.
    const char * get_name_cstring() const;

    /// @return RPM package Epoch as a `const char *`.
    /// @since 5.0
    /// @warning The value points to the underlying sack and shares its life-cycle.
    const char * get_epoch_cstring() const;

    /// @return RPM package Version as a `const char *`.
    /// @since 5.0
    /// @warning The value points to the underlying sack and shares its life-cycle.
    const char * get_version_cstring() const;

    /// @return RPM package Release as a `const char *`.
    /// @since 5.0
    /// @warning The value points to the underlying sack and shares its life-cycle.
    const char * get_release_cstring() const;

    /// @return RPM package Arch as a `const char *`.
    /// @since 5.0
    /// @warning The value points to the underlying sack and shares its life-cycle.
    const char * get_arch_cstring() const;

    /// @return RPM package EVR (Epoch:Version-Release) as a `const char *`. If the epoch is 0, it is omitted from the output.
    /// @since 5.0
    /// @warning The value points to the underlying sack and shares its life-cycle.
    const char * get_evr_cstring() const;

private:
    friend class PackageSetIterator;
    friend class PackageSack;
    friend class libdnf::Goal;
    friend class libdnf::base::Transaction;

    PackageSackWeakPtr sack;
    PackageId id;
};


inline Package::Package(const PackageSackWeakPtr & sack, PackageId id) : sack(sack), id(id) {}


inline bool Package::operator==(const Package & other) const noexcept {
    return id == other.id && sack == other.sack;
}


inline bool Package::operator!=(const Package & other) const noexcept {
    return id != other.id || sack != other.sack;
}

}  // namespace libdnf::rpm

#endif  // LIBDNF_RPM_PACKAGE_HPP
