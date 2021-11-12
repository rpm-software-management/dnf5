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

#ifndef LIBDNF_REPO_SOLV_REPO_HPP
#define LIBDNF_REPO_SOLV_REPO_HPP

#include "libdnf/base/base_weak.hpp"
#include "libdnf/common/exception.hpp"
#include "libdnf/repo/config_repo.hpp"

#include <solv/repo.h>


const constexpr int CHKSUM_BYTES = 32;


namespace libdnf::repo {

using LibsolvRepo = ::Repo;
enum class RepodataType { FILENAMES, PRESTO, UPDATEINFO, OTHER };
enum class RepodataState { NEW, LOADED_FETCH, LOADED_CACHE };

struct RepodataInfo {
    RepodataState state;
    Id id;
};


class SolvError : public Error {
    using Error::Error;

    const char * get_domain_name() const noexcept override { return "libdnf::repo"; }
    const char * get_name() const noexcept override { return "SolvError"; }
};


class SolvRepo {
public:
    SolvRepo(const libdnf::BaseWeakPtr & base, const ConfigRepo & config);

    /// Writes libsolv's .solv cache file with main libsolv repodata.
    void write_main(bool load_after_write);

    /// Writes libsolv's .solvx cache file with extended libsolv repodata.
    void write_ext(Id repodata_id, RepodataType type);

    /// Loads main metadata (solvables) from available repo.
    RepodataState load_repo_main(const std::string & repomd_fn, const std::string & primary_fn);

    /// Loads additional metadata (filelist, others, ...) from available repo.
    RepodataInfo load_repo_ext(const std::string & filename, RepodataType type);

    /// Loads system repository into the pool.
    ///
    /// @param rootdir If empty, loads the installroot rpmdb, if not loads rpmdb from this root path
    /// TODO(jrohel): Performance: Implement libsolv cache ("build_cache" argument) of system repo in future.
    bool load_system_repo(const std::string & rootdir = "");

    // Returns "true" when all solvables in the repository are stored contiguously -> No interleaving
    // with solvables from other repositories.
    // Complexity: Linear to the current number of solvables in  repository
    bool is_one_piece() const;

    // Internalize repository if needed.
    void internalize();

    // Checksum of data in .solv file. Used for validity check of .solvx files.
    unsigned char checksum[CHKSUM_BYTES];

    // the following three elements are needed for repo cache (.solv and .solvx updateinfo) writting
    int main_nsolvables{0};
    int main_nrepodata{0};
    int main_end{0};

    void set_needs_internalizing() { needs_internalizing = true; };

private:
    std::string solv_file_name(const char * type = nullptr);
    std::string solv_file_path(const char * type = nullptr);

    libdnf::BaseWeakPtr base;
    const ConfigRepo & config;

    bool needs_internalizing{false};

public:
    ::Repo * repo{nullptr};  // libsolv pool retains ownership
};

}  // namespace libdnf::repo

#endif  // LIBDNF_REPO_SOLV_REPO_HPP
