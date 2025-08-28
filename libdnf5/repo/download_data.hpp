// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_REPO_DOWNLOAD_DATA_HPP
#define LIBDNF5_REPO_DOWNLOAD_DATA_HPP

#include "librepo.hpp"
#include "repo_pgp.hpp"

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/repo/config_repo.hpp"
#include "libdnf5/repo/repo_callbacks.hpp"

#include <librepo/librepo.h>

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>


namespace libdnf5::repo {

class DownloadData {
public:
    DownloadData(const libdnf5::BaseWeakPtr & base, const ConfigRepo & config, Repo::Type repo_type);
    ~DownloadData();

    void reset_loaded();

    void set_callbacks(std::unique_ptr<libdnf5::repo::RepoCallbacks> && callbacks) noexcept;
    void set_user_data(void * user_data) noexcept;
    void * get_user_data() const noexcept;

    const std::string & get_metadata_path(const std::string & metadata_type) const;
    std::vector<std::pair<std::string, std::string>> get_appstream_metadata() const;

private:
    friend class Repo;
    friend class RepoSack;
    friend class RepoDownloader;

    std::pair<std::string, std::string> get_source_info() const;
    std::set<std::string> get_optional_metadata() const;
    bool is_appstream_metadata_type(const std::string & type) const;

    libdnf5::BaseWeakPtr base;
    const ConfigRepo & config;
    Repo::Type repo_type;
    RepoPgp pgp;

    std::unique_ptr<RepoCallbacks> callbacks;
    void * user_data{nullptr};

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

}  // namespace libdnf5::repo

#endif  // LIBDNF5_REPO_DOWNLOAD_DATA_HPP
