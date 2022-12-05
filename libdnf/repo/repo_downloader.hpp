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

#ifndef LIBDNF_REPO_REPO_DOWNLOADER_HPP
#define LIBDNF_REPO_REPO_DOWNLOADER_HPP

#include "librepo.hpp"
#include "repo_pgp.hpp"

#include "libdnf/base/base_weak.hpp"
#include "libdnf/common/exception.hpp"
#include "libdnf/repo/config_repo.hpp"
#include "libdnf/repo/repo.hpp"
#include "libdnf/repo/repo_callbacks.hpp"

#include <librepo/librepo.h>

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>


namespace libdnf::repo {

/// Handles downloading and loading of repository metadata.
/// @exception RepoDownloadError (public) All public methods should throw this exception,
///                                       under which a lower-level exception can often be nested.
class RepoDownloader {
public:
    // Names of metadata files in rpm repository
    // Final metadata file name is (hash-) + this constant + ".xml" [+ compression suffix]
    static constexpr const char * MD_FILENAME_PRIMARY = "primary";
    static constexpr const char * MD_FILENAME_FILELISTS = "filelists";
    static constexpr const char * MD_FILENAME_PRESTODELTA = "prestodelta";
    static constexpr const char * MD_FILENAME_UPDATEINFO = "updateinfo";
    static constexpr const char * MD_FILENAME_OTHER = "other";
    static constexpr const char * MD_FILENAME_GROUP_GZ = "group_gz";
    static constexpr const char * MD_FILENAME_GROUP = "group";
    static constexpr const char * MD_FILENAME_MODULES = "modules";

    RepoDownloader(const libdnf::BaseWeakPtr & base, const ConfigRepo & config, Repo::Type repo_type);

    ~RepoDownloader();

    void download_metadata(const std::string & destdir);
    bool is_metalink_in_sync();
    bool is_repomd_in_sync();
    void load_local();

    LibrepoHandle & get_cached_handle();

    void set_callbacks(std::unique_ptr<libdnf::repo::RepoCallbacks> && callbacks) noexcept;

    const std::string & get_metadata_path(const std::string & metadata_type) const;

private:
    friend class Repo;

    LibrepoHandle init_local_handle();
    LibrepoHandle init_remote_handle(const char * destdir, bool mirror_setup = true);
    void common_handle_setup(LibrepoHandle & h);

    void apply_http_headers(LibrepoHandle & handle);

    LibrepoResult perform(LibrepoHandle & handle, const std::string & dest_directory, bool set_gpg_home_dir);

    void download_url(const char * url, int fd);

    std::pair<std::string, std::string> get_source_info() const;

    void import_repo_keys();

    std::string get_persistdir() const;
    void add_countme_flag(LibrepoHandle & handle);

    std::unordered_set<std::string> get_optional_metadata() const;

    libdnf::BaseWeakPtr base;
    const ConfigRepo & config;
    Repo::Type repo_type;
    RepoPgp pgp;

    std::unique_ptr<RepoCallbacks> callbacks;

    // download input
    bool preserve_remote_time = false;
    int max_mirror_tries = 0;  // try all mirrors
    std::map<std::string, std::string> substitutions;
    std::vector<std::string> http_headers;

    // download output
    std::string repomd_filename;
    std::vector<std::string> mirrors;
    std::string revision;
    int max_timestamp{0};
    std::vector<std::string> content_tags;
    std::vector<std::pair<std::string, std::string>> distro_tags;
    std::vector<std::pair<std::string, std::string>> metadata_locations;
    std::map<std::string, std::string> metadata_paths;

    std::optional<LibrepoHandle> handle;
};


// TODO(lukash) if we want this, create a place for this downloader on the API
//struct Downloader {
//public:
//    static void download_url(ConfigMain * cfg, const char * url, int fd);
//};

}  // namespace libdnf::repo

#endif  // LIBDNF_REPO_REPO_DOWNLOADER_HPP
