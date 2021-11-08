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

#ifndef LIBDNF_RPM_REPO_IMPL_HPP
#define LIBDNF_RPM_REPO_IMPL_HPP

#include "repo_downloader.hpp"
#include "libdnf/base/base.hpp"
#include "libdnf/repo/repo.hpp"

#include <gpgme.h>
#include <solv/chksum.h>
#include <solv/repo.h>
#include <solv/util.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <map>
#include <mutex>
#include <set>


#define CHKSUM_BYTES 32

namespace libdnf::repo {

using LibsolvRepo = ::Repo;

// Information about attached libsolv repository
class LibsolvRepoExt {
public:
    // Returns "true" when all solvables in the repository are stored contiguously -> No interleaving
    // with solvables from other repositories.
    // Complexity: Linear to the current number of solvables in  repository
    bool is_one_piece() const;

    // Internalize repository if needed.
    void internalize();

    LibsolvRepo * repo{nullptr};

    // Checksum of data in .solv file. Used for validity check of .solvx files.
    unsigned char checksum[CHKSUM_BYTES];

    // the following three elements are needed for repo cache (.solv and .solvx updateinfo) writting
    int main_nsolvables{0};
    int main_nrepodata{0};
    int main_end{0};

    void set_needs_internalizing() { needs_internalizing = true; };

private:
    bool needs_internalizing{false};
};


class Repo::Impl {
public:
    // Names of well known metadata files in rpm repository
    // Final metadata file name is (hash-) + this constant + ".xml" [+ compression suffix]
    static constexpr const char * MD_FILENAME_PRIMARY = "primary";
    static constexpr const char * MD_FILENAME_FILELISTS = "filelists";
    static constexpr const char * MD_FILENAME_PRESTODELTA = "prestodelta";
    static constexpr const char * MD_FILENAME_UPDATEINFO = "updateinfo";
    static constexpr const char * MD_FILENAME_OTHER = "other";
    static constexpr const char * MD_FILENAME_GROUP_GZ = "group_gz";
    static constexpr const char * MD_FILENAME_GROUP = "group";
    static constexpr const char * MD_FILENAME_MODULES = "modules";

    Impl(const BaseWeakPtr & base, Repo & owner, std::string id, Type type);
    ~Impl();

    bool load();
    void load_cache();
    bool try_load_cache();
    bool is_in_sync();
    int64_t get_age() const;
    void expire();
    bool is_expired() const;
    int get_expires_in() const;
    const std::string & get_metadata_path(const std::string & metadata_type) const;

    void attach_libsolv_repo(LibsolvRepo * libsolv_repo);
    void detach_libsolv_repo();

    /// When add_with_hdrid == true the rpm is loaded with additional flags (RPM_ADD_WITH_HDRID|RPM_ADD_WITH_SHA256SUM)
    /// It will calculate SHA256 checksum of header and store it in pool => Requires more CPU for loading
    /// When RPM is not accesible or corrupted it raises libdnf::RuntimeError
    /// Return Id of a new solvable
    Id add_rpm_package(const std::string & fn, bool add_with_hdrid);

public:
    friend class Repo;
    Type type;
    ConfigRepo config;

    std::vector<std::string> mirrors;
    // 0 forces expiration on the next call to load(), -1 means undefined value
    int64_t timestamp;
    int max_timestamp{0};
    std::string revision;
    std::vector<std::string> content_tags;
    std::vector<std::pair<std::string, std::string>> distro_tags;
    std::vector<std::pair<std::string, std::string>> metadata_locations;
    bool use_includes{false};

    std::string repo_file_path;

    SyncStrategy sync_strategy;
    std::map<std::string, std::string> metadata_paths;

    Repo * owner;
    libdnf::BaseWeakPtr base;
    void reset_metadata_expired();

    bool expired;
    static bool ends_with(std::string const & str, std::string const & ending);

    // Information about attached libsolv repository
    LibsolvRepoExt libsolv_repo_ext;

    RepoDownloader downloader;
};

}  // namespace libdnf::repo

#endif  // LIBDNF_RPM_REPO_IMPL_HPP
