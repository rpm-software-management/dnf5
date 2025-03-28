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

#ifndef LIBDNF5_REPO_REPO_DOWNLOADER_HPP
#define LIBDNF5_REPO_REPO_DOWNLOADER_HPP


#include "repo/download_data.hpp"


namespace libdnf5::repo {

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
    static constexpr const char * MD_FILENAME_APPSTREAM = "appstream";

    //TODO(amatej): make these non static and check all added repos metalink/repomd in parallel
    static bool is_metalink_in_sync(Repo & repo);
    static bool is_repomd_in_sync(Repo & repo);

    static void load_local(DownloadData & download_data);
    static LibrepoHandle & get_cached_handle(Repo & repo);

    void download_metadata(const std::string & destdir);


private:
    LibrepoResult perform(LibrepoHandle & handle, bool set_gpg_home_dir);

    struct CallbackData {
        void * user_cb_data{nullptr};
        double prev_total_to_download;
        double prev_downloaded;
        double sum_prev_downloaded;
    };

    static LibrepoHandle init_local_handle(const DownloadData & download_data);
    static LibrepoHandle init_remote_handle(Repo & repo, const char * destdir, bool mirror_setup = true);
    static void common_handle_setup(LibrepoHandle & h, const DownloadData & download_data);
    static void apply_http_headers(DownloadData & download_data, LibrepoHandle & handle);
    static void add_countme_flag(DownloadData & download_data, LibrepoHandle & handle);
    static time_t get_system_epoch();

    static int end_cb(void * data, LrTransferStatus status, const char * msg);
    static int progress_cb(void * data, double total_to_download, double downloaded);
    static void fastest_mirror_cb(void * data, LrFastestMirrorStages stage, void * ptr);
    static int mirror_failure_cb(void * data, const char * msg, const char * url);
    static int mirror_failure_cb(void * data, const char * msg, const char * url, const char * metadata);

};

}  // namespace libdnf5::repo

#endif  // LIBDNF5_REPO_REPO_DOWNLOADER_HPP
