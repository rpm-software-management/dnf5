/*
Copyright (C) 2020 Red Hat, Inc.

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


#ifndef LIBDNF_RPM_PACKAGE_HPP
#define LIBDNF_RPM_PACKAGE_HPP

#include "checksum.hpp"
#include "reldep_list.hpp"
#include "solv_sack.hpp"

#include <string>
#include <vector>


namespace libdnf::rpm {


class PackageSetIterator;


/// @replaces libdnf/hy-package.h:struct:DnfPackage
/// @replaces dnf:dnf/package.py:class:Package
class Package {
public:
    /// @replaces libdnf/hy-package.h:function:dnf_package_get_identical(DnfPackage * pkg)
    bool operator==(const Package & other) const noexcept;
    bool operator!=(const Package & other) const noexcept;

    /// @replaces libdnf/hy-package.h:function:dnf_package_get_id(DnfPackage * pkg)
    PackageId get_id() const noexcept { return id; };

    /// @replaces dnf:dnf/package.py:attribute:Package.name
    /// @replaces libdnf/hy-package.h:function:dnf_package_get_name(DnfPackage *pkg);
    std::string get_name() const;

    /// @replaces dnf:dnf/package.py:attribute:Package.e
    /// @replaces dnf:dnf/package.py:attribute:Package.epoch
    /// @replaces libdnf/hy-package.h:function:dnf_package_get_epoch(DnfPackage *pkg);
    unsigned long get_epoch();

    /// @replaces dnf:dnf/package.py:attribute:Package.v
    /// @replaces dnf:dnf/package.py:attribute:Package.version
    /// @replaces libdnf/hy-package.h:function:dnf_package_get_version(DnfPackage *pkg);
    std::string get_version();

    /// @replaces dnf:dnf/package.py:attribute:Package.r
    /// @replaces dnf:dnf/package.py:attribute:Package.release
    /// @replaces libdnf/hy-package.h:function:dnf_package_get_release(DnfPackage *pkg);
    std::string get_release();

    /// @replaces dnf:dnf/package.py:attribute:Package.a
    /// @replaces dnf:dnf/package.py:attribute:Package.arch
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_arch(DnfPackage * pkg)
    std::string get_arch() const;

    /// @replaces dnf:dnf/package.py:attribute:Package.evr
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_evr(DnfPackage * pkg)
    std::string get_evr() const;

    /// Reurn nevra withou epoch when epoch is 0
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_nevra(DnfPackage * pkg)
    std::string get_nevra();

    /// Reurn always nevra with epoch
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_nevra(DnfPackage * pkg)
    std::string get_full_nevra();

    /// @replaces dnf:dnf/package.py:attribute:Package.group
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_group(DnfPackage * pkg)
    std::string get_group();

    /// If package is installed, return get_install_size(). Return get_download_size() otherwise.
    /// @replaces dnf:dnf/package.py:attribute:Package.size
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_size(DnfPackage * pkg)
    unsigned long long get_size() noexcept;

    /// Return size of an RPM package: <size package="..."/>
    /// @replaces dnf:dnf/package.py:attribute:Package.downloadsize
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_downloadsize(DnfPackage * pkg)
    unsigned long long get_download_size() noexcept;

    /// Return size of an RPM package installed on a system: <size installed="..."/>
    /// @replaces dnf:dnf/package.py:attribute:Package.installsize
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_installsize(DnfPackage * pkg)
    unsigned long long get_install_size() noexcept;

    /// @replaces dnf:dnf/package.py:attribute:Package.license
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_license(DnfPackage * pkg)
    std::string get_license();

    /// @replaces dnf:dnf/package.py:attribute:Package.sourcerpm
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_sourcerpm(DnfPackage * pkg)
    std::string get_sourcerpm();

    /// @replaces dnf:dnf/package.py:attribute:Package.buildtime
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_buildtime(DnfPackage * pkg)
    unsigned long long get_build_time() noexcept;

    // TODO not supported by libsolv: https://github.com/openSUSE/libsolv/issues/400
    //std::string get_build_host();

    /// @replaces dnf:dnf/package.py:attribute:Package.packager
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_packager(DnfPackage * pkg)
    std::string get_packager();

    std::string get_vendor();

    /// @replaces dnf:dnf/package.py:attribute:Package.url
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_url(DnfPackage * pkg)
    std::string get_url();

    /// @replaces dnf:dnf/package.py:attribute:Package.summary
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_summary(DnfPackage * pkg)
    std::string get_summary();

    /// @replaces dnf:dnf/package.py:attribute:Package.description
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_description(DnfPackage * pkg)
    std::string get_description();

    /// @replaces dnf:dnf/package.py:attribute:Package.files
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_files(DnfPackage * pkg)
    /// TODO(dmach): files, directories, info about ghost etc. - existing implementation returns incomplete data
    std::vector<std::string> get_files();

    // DEPENDENCIES

    /// @replaces dnf:dnf/package.py:attribute:Package.provides
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_provides(DnfPackage * pkg)
    ReldepList get_provides() const;

    /// @replaces dnf:dnf/package.py:attribute:Package.requires
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_requires(DnfPackage * pkg)
    ReldepList get_requires() const;

    /// @replaces dnf:dnf/package.py:attribute:Package.requires_pre
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_requires_pre(DnfPackage * pkg)
    ReldepList get_requires_pre() const;

    /// @replaces dnf:dnf/package.py:attribute:Package.conflicts
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_conflicts(DnfPackage * pkg)
    ReldepList get_conflicts() const;

    /// @replaces dnf:dnf/package.py:attribute:Package.obsoletes
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_obsoletes(DnfPackage * pkg)
    ReldepList get_obsoletes() const;

    /// @replaces dnf:dnf/package.py:attribute:Package.prereq_ignoreinst
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_prereq_ignoreinst(DnfPackage * pkg)
    ReldepList get_prereq_ignoreinst() const;

    /// @replaces dnf:dnf/package.py:attribute:Package.regular_requires
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_regular_requires(DnfPackage * pkg)
    ReldepList get_regular_requires() const;

    // WEAK DEPENDENCIES

    /// @replaces dnf:dnf/package.py:attribute:Package.recommends
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_recommends(DnfPackage * pkg)
    ReldepList get_recommends() const;

    /// @replaces dnf:dnf/package.py:attribute:Package.suggests
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_suggests(DnfPackage * pkg)
    ReldepList get_suggests() const;

    /// @replaces dnf:dnf/package.py:attribute:Package.enhances
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_enhances(DnfPackage * pkg)
    ReldepList get_enhances() const;

    /// @replaces dnf:dnf/package.py:attribute:Package.supplements
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_supplements(DnfPackage * pkg)
    ReldepList get_supplements() const;

    // REPODATA

    /// @replaces dnf:dnf/package.py:attribute:Package.baseurl
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_baseurl(DnfPackage * pkg)
    std::string get_baseurl();

    /// @replaces dnf:dnf/package.py:attribute:Package.location
    /// @replaces dnf:dnf/package.py:attribute:Package.relativepath
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_location(DnfPackage * pkg)
    std::string get_location();

    /// Returns package location (file path) in the filesystem.
    /// For packages in remote repo returns where the package will be/has been downloaded.
    /// @replaces dnf:dnf/package.py:attribute:Package.localPkg
    std::string get_local_filepath() const;

    /// @replaces dnf:dnf/package.py:attribute:Package.chksum
    /// @replaces libdnf:libdnf/hy-package-private.hpp:function:dnf_package_get_chksum(DnfPackage *pkg, int *type)
    Checksum get_checksum();

    /// @replaces dnf:dnf/package.py:attribute:Package.hdr_chksum
    /// @replaces libdnf:libdnf/hy-package-private.hpp:function:dnf_package_get_hdr_chksum(DnfPackage *pkg, int *type)
    Checksum get_hdr_checksum();

    /// TODO get_changelogs - requires changelog
    /// @replaces dnf:dnf/package.py:attribute:Package.changelogs
    /// @replaces libdnf:libdnf/hy-package-private.hpp:function:dnf_package_get_changelogs(DnfPackage * pkg)
    // void get_changelogs() const;

    // SYSTEM

    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_installed(DnfPackage * pkg)
    /// @replaces dnf:dnf/package.py:attribute:Package.installed
    bool is_installed() const;

    /// TODO is_local
    /// @replaces dnf:dnf/package.py:method:Package.localPkg(self)
    /// @replaces libdnf:libdnf/dnf-package.h:function:dnf_package_is_local(DnfPackage * pkg)
    //bool is_local() const;

    /// TODO get_from_repo_id - requires swdb
    /// For an installed package, return repoid of repo from the package was installed.
    /// For an available package, return an empty string.
    ///
    /// @replaces dnf:dnf/package.py:attribute:Package.ui_from_repo
    /// @replaces libdnf:libdnf/dnf-package.h:function:dnf_package_get_origin(DnfPackage * pkg)
    // void get_from_repo_id() const;

    /// @replaces dnf:dnf/package.py:attribute:Package.hdr_end
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_hdr_end(DnfPackage * pkg)
    unsigned long long get_hdr_end() noexcept;

    /// @replaces dnf:dnf/package.py:attribute:Package.installtime
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_installtime(DnfPackage * pkg)
    unsigned long long get_install_time() noexcept;

    /// @brief Media number for the package
    ///
    /// @replaces dnf:dnf/package.py:attribute:Package.medianr
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_medianr(DnfPackage * pkg)
    unsigned long long get_media_number() noexcept;

    /// @brief The rpmdb ID for the package
    ///
    /// @replaces dnf:dnf/package.py:attribute:Package.rpmdbid
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_rpmdbid(DnfPackage * pkg)
    unsigned long long get_rpmdbid() const noexcept;

    /// TODO get_repo - requires Repo
    /// @replaces dnf:dnf/package.py:attribute:Package.repo
    /// @replaces dnf:dnf/package.py:attribute:Package.repoid
    /// @replaces dnf:dnf/package.py:attribute:Package.reponame
    /// @replaces libdnf:libdnf/dnf-package.h:function:dnf_package_get_repo(DnfPackage * pkg)
    /// @replaces libdnf:libdnf/hy-package.h:function:dnf_package_get_reponame(DnfPackage * pkg)
    Repo * get_repo() const noexcept;

    // TODO(dmach): getBugUrl() not possible due to lack of support in libsolv and metadata?

protected:
    /// @replaces libdnf:libdnf/dnf-package.h:function:dnf_package_new(DnfSack *sack, Id id)
    Package(SolvSack * sack, PackageId id);
    const char * get_name_cstring() const noexcept;

    /// @return const char* !! Return temporal values !!
    const char * get_epoch_cstring();

    /// @return const char* !! Return temporal values !!
    const char * get_version_cstring() noexcept;

    /// @return const char* !! Return temporal values !!
    const char * get_release_cstring() noexcept;

    const char * get_arch_cstring() const noexcept;

    const char * get_evr_cstring() const noexcept;

private:
    friend PackageSetIterator;
    SolvSackWeakPtr sack;
    PackageId id;
};

inline Package::Package(SolvSack * sack, PackageId id) : sack(sack->get_weak_ptr()), id(id) {}

inline bool Package::operator==(const Package & other) const noexcept {
    return id == other.id && sack == other.sack;
}

inline bool Package::operator!=(const Package & other) const noexcept {
    return id != other.id || sack != other.sack;
}

}  // namespace libdnf::rpm

#endif  // LIBDNF_RPM_PACKAGE_HPP
