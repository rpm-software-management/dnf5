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
#include "solv_repo.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/repo/repo.hpp"

#include <gpgme.h>
#include <solv/chksum.h>
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


namespace libdnf::repo {

using LibsolvRepo = ::Repo;

class Repo::Impl {
public:
    Impl(const BaseWeakPtr & base, std::string id, Type type);
    ~Impl();

    bool fetch_metadata();
    void read_metadata_cache();
    bool try_load_cache();
    bool is_in_sync();
    int64_t get_age() const;
    void expire();
    bool is_expired() const;
    int get_expires_in() const;
    const std::string & get_metadata_path(const std::string & metadata_type) const;

    void load_available_repo(LoadFlags flags);

    /// When add_with_hdrid == true the rpm is loaded with additional flags (RPM_ADD_WITH_HDRID|RPM_ADD_WITH_SHA256SUM)
    /// It will calculate SHA256 checksum of header and store it in pool => Requires more CPU for loading
    /// When RPM is not accesible or corrupted it raises libdnf::RuntimeError
    /// Return Id of a new solvable
    Id add_rpm_package(const std::string & fn, bool add_with_hdrid);

public:
    friend class Repo;
    Type type;
    ConfigRepo config;

    // 0 forces expiration on the next call to load(), -1 means undefined value
    int64_t timestamp;
    bool use_includes{false};

    std::string repo_file_path;

    SyncStrategy sync_strategy;

    libdnf::BaseWeakPtr base;
    void reset_metadata_expired();

    bool expired;
    static bool ends_with(std::string const & str, std::string const & ending);

    // Information about attached libsolv repository
    // TODO(lukash) create solv_repo only when loading into the pool (make this std::optional or std::unique_ptr)
    // and handle the case throughout the code via asserts
    SolvRepo solv_repo;

    RepoDownloader downloader;
};

}  // namespace libdnf::repo

#endif  // LIBDNF_RPM_REPO_IMPL_HPP
