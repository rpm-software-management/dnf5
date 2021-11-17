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

#include "libdnf/base/base_weak.hpp"
#include "libdnf/common/exception.hpp"
#include "libdnf/repo/config_repo.hpp"
#include "libdnf/repo/repo_callbacks.hpp"
#include "libdnf/repo/repo_gpgme.hpp"

#include <librepo/librepo.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <gpgme.h>


namespace std {

template <>
struct default_delete<LrHandle> {
    void operator()(LrHandle * ptr) noexcept { lr_handle_free(ptr); }
};

template <>
struct default_delete<LrResult> {
    void operator()(LrResult * ptr) noexcept { lr_result_free(ptr); }
};

}  // namespace std


namespace libdnf::repo {

class LibrepoError : public Error {
public:
    LibrepoError(std::unique_ptr<GError> && lr_error);
    const char * get_domain_name() const noexcept override { return "libdnf::repo"; }
    const char * get_name() const noexcept override { return "LibrepoError"; }
    int get_code() const noexcept { return code; }

private:
    int code;
};


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

    RepoDownloader(const libdnf::BaseWeakPtr & base, const ConfigRepo & config);

    ~RepoDownloader();

    void download_metadata(const std::string & destdir);
    bool is_metalink_in_sync();
    bool is_repomd_in_sync();
    void load_local();

    LrHandle * get_cached_handle();

    std::vector<std::string> get_mirrors() const noexcept { return mirrors; }

    std::string get_repomd_filename() const noexcept { return repomd_filename; }
    std::string get_revision() const;
    int get_max_timestamp() const;
    std::map<std::string, std::string> get_metadata_paths() const;
    std::vector<std::string> get_content_tags() const;
    std::vector<std::pair<std::string, std::string>> get_distro_tags() const;
    std::vector<std::pair<std::string, std::string>> get_metadata_locations() const;

    void set_callbacks(std::unique_ptr<libdnf::repo::RepoCallbacks> && callbacks) noexcept;

private:
    friend class Repo;

    std::unique_ptr<LrHandle> init_local_handle();
    std::unique_ptr<LrHandle> init_remote_handle(const char * destdir, bool mirror_setup = true);
    void common_handle_setup(std::unique_ptr<LrHandle> & h);

    void apply_http_headers(std::unique_ptr<LrHandle> & handle);

    std::unique_ptr<LrResult> perform(LrHandle * handle, const std::string & dest_directory, bool set_gpg_home_dir);

    void download_url(const char * url, int fd);

    std::pair<std::string, std::string> get_source_info() const;

    void import_repo_keys();

    std::string get_persistdir() const;
    void add_countme_flag(LrHandle * handle);

    libdnf::BaseWeakPtr base;
    const ConfigRepo & config;
    RepoGpgme gpgme;

    std::unique_ptr<RepoCallbacks> callbacks;

    bool load_metadata_other = false;
    std::set<std::string> additional_metadata;
    bool preserve_remote_time = false;
    int max_mirror_tries = 0;  // try all mirrors
    std::map<std::string, std::string> substitutions;
    std::vector<std::string> http_headers;

    std::string repomd_filename;

    std::unique_ptr<LrResult> lr_result;

    std::unique_ptr<LrHandle> handle;

    std::vector<std::string> mirrors;
};


// TODO(lukash) if we want this, create a place for this downloader on the API
//struct Downloader {
//public:
//    static void download_url(ConfigMain * cfg, const char * url, int fd);
//};

}  // namespace libdnf::repo

#endif  // LIBDNF_REPO_REPO_DOWNLOADER_HPP
