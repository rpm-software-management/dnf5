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

#ifndef LIBDNF5_REPO_SOLV_REPO_HPP
#define LIBDNF5_REPO_SOLV_REPO_HPP

#include "repo_downloader.hpp"
#include "solv/id_queue.hpp"
#include "solv/pool.hpp"

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/common/exception.hpp"
#include "libdnf5/repo/config_repo.hpp"
#include "libdnf5/utils/fs/file.hpp"

#include <solv/repo.h>

#include <filesystem>


static const constexpr size_t CHKSUM_BYTES = 32;
static const constexpr size_t SOLV_USERDATA_SOLV_TOOLVERSION_SIZE{8};
static const constexpr std::array<char, 4> SOLV_USERDATA_MAGIC{'\0', 'd', 'n', 'f'};
static const constexpr std::array<char, 4> SOLV_USERDATA_DNF_VERSION{'\0', '1', '.', '0'};

static constexpr const size_t SOLV_USERDATA_SIZE =
    SOLV_USERDATA_SOLV_TOOLVERSION_SIZE + SOLV_USERDATA_MAGIC.size() + SOLV_USERDATA_DNF_VERSION.size() + CHKSUM_BYTES;

struct SolvUserdata {
    char dnf_magic[SOLV_USERDATA_MAGIC.size()];
    char dnf_version[SOLV_USERDATA_DNF_VERSION.size()];
    char libsolv_version[SOLV_USERDATA_SOLV_TOOLVERSION_SIZE];
    unsigned char checksum[CHKSUM_BYTES];
} __attribute__((packed));

namespace libdnf5::repo {

using LibsolvRepo = ::Repo;
enum class RepodataType { FILELISTS, PRESTO, UPDATEINFO, COMPS, OTHER };


class SolvError : public Error {
    using Error::Error;

    const char * get_domain_name() const noexcept override { return "libdnf5::repo"; }
    const char * get_name() const noexcept override { return "SolvError"; }
};


class SolvRepo {
public:
    SolvRepo(const libdnf5::BaseWeakPtr & base, const ConfigRepo & config, void * appdata);
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

    /// Loads additional system repo metadata (comps, modules)
    void load_system_repo_ext(RepodataType type);

    void rewrite_repo(libdnf5::solv::IdQueue & fileprovides);

    // Internalize repository if needed.
    void internalize();

    void set_priority(int priority);
    void set_subpriority(int subpriority);

    // Checksum of data in .solv file. Used for validity check of .solvx files.
    unsigned char checksum[CHKSUM_BYTES];

    void set_needs_internalizing() { needs_internalizing = true; };

    /// @return  Vector of group ids of system repo groups without valid xml
    std::vector<std::string> & get_groups_missing_xml() { return groups_missing_xml; };

    /// @return  Vector of environment ids of system repo environmental groups without valid xml
    std::vector<std::string> & get_environments_missing_xml() { return environments_missing_xml; };

    /// Create a group solvable based on what's available in system state. Used in
    /// case we are not able to load metadata from xml file.
    /// @param groupid  Id of the group
    /// @param state    group state from the system state
    void create_group_solvable(const std::string & groupid, const libdnf5::system::GroupState & state);

    /// Create an environmental group solvable based on what's available in
    /// system state. Used in case we are not able to load metadata from xml file.
    /// @param environmentid  Id of the environment
    /// @param state    environment state from the system state
    void create_environment_solvable(
        const std::string & environmentid, const libdnf5::system::EnvironmentState & state);

    /// Read comps group solvable from its xml file.
    /// @param path  Path to xml file.
    /// @return      True if the group was successfully read.
    bool read_group_solvable_from_xml(const std::string & path);

private:
    // "type_name == nullptr" means load "primary" cache (.solv file)
    bool load_solv_cache(solv::Pool & pool, const char * type_name, int flags);

    /// Writes libsolv's .solv cache file with main libsolv repodata.
    void write_main(bool load_after_write);

    /// Writes libsolv's .solvx cache file with extended libsolv repodata.
    void write_ext(Id repodata_id, RepodataType type);

    std::string solv_file_name(const char * type = nullptr);
    std::filesystem::path solv_file_path(const char * type = nullptr);

    libdnf5::BaseWeakPtr base;
    const ConfigRepo & config;

    bool needs_internalizing{false};

    /// Ranges of solvables for different types of data, used for writing libsolv cache files
    int main_solvables_start{0};
    int main_solvables_end{0};

    /// Range of repodata from the primary.xml, used for writing libsolv cache files
    int main_repodata_start{0};
    int main_repodata_end{0};

    int updateinfo_solvables_start{0};
    int updateinfo_solvables_end{0};

    bool can_use_solvfile_cache(solv::Pool & pool, utils::fs::File & solvfile_cache);
    void userdata_fill(SolvUserdata * userdata);

    /// List of system repo groups without valid file with xml definition
    std::vector<std::string> groups_missing_xml;

    /// List of system repo environmental groups without valid file with xml definition
    std::vector<std::string> environments_missing_xml;

public:
    ::Repo * repo{nullptr};  // libsolv pool retains ownership
    // Solvables for groups and environments are kept in separate pool. It means
    // we need also separate Repo object created in that pool.
    ::Repo * comps_repo{nullptr};
};

}  // namespace libdnf5::repo

#endif  // LIBDNF5_REPO_SOLV_REPO_HPP
