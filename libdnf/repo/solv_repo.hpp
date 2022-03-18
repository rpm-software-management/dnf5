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

#include "repo_downloader.hpp"
#include "solv/id_queue.hpp"

#include "libdnf/base/base_weak.hpp"
#include "libdnf/common/exception.hpp"
#include "libdnf/repo/config_repo.hpp"

#include <solv/repo.h>


const constexpr int CHKSUM_BYTES = 32;


namespace libdnf::repo {

using LibsolvRepo = ::Repo;
enum class RepodataType { FILELISTS, PRESTO, UPDATEINFO, COMPS, OTHER };


class SolvError : public Error {
    using Error::Error;

    const char * get_domain_name() const noexcept override { return "libdnf::repo"; }
    const char * get_name() const noexcept override { return "SolvError"; }
};


class SolvRepo {
public:
    SolvRepo(const libdnf::BaseWeakPtr & base, const ConfigRepo & config, void * appdata);
    ~SolvRepo();

    /// Loads main metadata (solvables) from available repo.
    void load_repo_main(const std::string & repomd_fn, const std::string & primary_fn);

    /// Loads additional metadata (filelist, others, ...) from available repo.
    void load_repo_ext(RepodataType type, const RepoDownloader & downloader);

    /// Loads system repository into the pool.
    ///
    /// @param rootdir If empty, loads the installroot rpmdb, if not loads rpmdb from this root path
    /// TODO(jrohel): Performance: Implement libsolv cache ("build_cache" argument) of system repo in future.
    void load_system_repo(const std::string & rootdir = "");

    void rewrite_repo(libdnf::solv::IdQueue & fileprovides);

    // Internalize repository if needed.
    void internalize();

    void set_priority(int priority);
    void set_subpriority(int subpriority);

    // Checksum of data in .solv file. Used for validity check of .solvx files.
    unsigned char checksum[CHKSUM_BYTES];

    void set_needs_internalizing() { needs_internalizing = true; };

private:
    bool load_solv_cache(const char * type, int flags);

    /// Writes libsolv's .solv cache file with main libsolv repodata.
    void write_main();

    /// Writes libsolv's .solvx cache file with extended libsolv repodata.
    void write_ext(Id repodata_id, RepodataType type);

    std::string solv_file_name(const char * type = nullptr);
    std::string solv_file_path(const char * type = nullptr);

    libdnf::BaseWeakPtr base;
    const ConfigRepo & config;

    bool needs_internalizing{false};

    /// Ranges of solvables for different types of data, used for writing libsolv cache files
    int main_solvables_start{0};
    int main_solvables_end{0};
    int updateinfo_solvables_start{0};
    int updateinfo_solvables_end{0};

public:
    ::Repo * repo{nullptr};  // libsolv pool retains ownership
};

}  // namespace libdnf::repo

#endif  // LIBDNF_REPO_SOLV_REPO_HPP
