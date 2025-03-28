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

#include "download_data.hpp"

#include "utils/string.hpp"

#include "libdnf5/conf/const.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <solv/chksum.h>
#include <solv/util.h>
#include <sys/stat.h>


namespace libdnf5::repo {


DownloadData::DownloadData(const libdnf5::BaseWeakPtr & base, const ConfigRepo & config, Repo::Type repo_type)
    : base(base),
      config(config),
      repo_type(repo_type),
      pgp(base, config) {}

DownloadData::~DownloadData() = default;

void DownloadData::reset_loaded() {
    repomd_filename.clear();
    mirrors.clear();
    revision.clear();
    max_timestamp = 0;
    content_tags.clear();
    distro_tags.clear();
    metadata_locations.clear();
    metadata_paths.clear();
}

void DownloadData::set_callbacks(std::unique_ptr<libdnf5::repo::RepoCallbacks> && cbs) noexcept {
    callbacks = std::move(cbs);
    pgp.set_callbacks(callbacks.get());
}

void DownloadData::set_user_data(void * user_data) noexcept {
    this->user_data = user_data;
}

void * DownloadData::get_user_data() const noexcept {
    return user_data;
}

const std::string & DownloadData::get_metadata_path(const std::string & metadata_type) const {
    auto it = metadata_paths.end();

    if (config.get_main_config().get_zchunk_option().get_value() && !utils::string::ends_with(metadata_type, "_zck")) {
        it = metadata_paths.find(metadata_type + "_zck");
    }

    if (it == metadata_paths.end()) {
        it = metadata_paths.find(metadata_type);
    }

    static const std::string empty;
    return it != metadata_paths.end() ? it->second : empty;
}

bool DownloadData::is_appstream_metadata_type(const std::string & type) const {
    /* TODO: make the list configurable with this default */
    return utils::string::starts_with(type, "appstream") || utils::string::starts_with(type, "appdata");
}

std::vector<std::pair<std::string, std::string>> DownloadData::get_appstream_metadata() const {
    std::vector<std::pair<std::string, std::string>> appstream_metadata;
    /* The RepoDownloader::common_handle_setup() sets the expected names,
       check for the starts_with() only here, to avoid copying the list here. */

    for (auto it = metadata_paths.begin(); it != metadata_paths.end(); it++) {
        const std::string type = it->first;
        const std::string path = it->second;

        if (is_appstream_metadata_type(type))
            appstream_metadata.push_back(std::pair<std::string, std::string>(type, path));
    }
    return appstream_metadata;
}

std::pair<std::string, std::string> DownloadData::get_source_info() const {
    if (!config.get_metalink_option().empty() && !config.get_metalink_option().get_value().empty()) {
        return {"metalink", config.get_metalink_option().get_value()};
    } else if (!config.get_mirrorlist_option().empty() && !config.get_mirrorlist_option().get_value().empty()) {
        return {"mirrorlist", config.get_mirrorlist_option().get_value()};
    } else {
        return {"baseurl", libdnf5::utils::string::join(config.get_baseurl_option().get_value(), ", ")};
    }
}

// TODO(jkolarik): currently all metadata are loaded for system repo, maybe we want it configurable?
std::set<std::string> DownloadData::get_optional_metadata() const {
    if (repo_type == Repo::Type::SYSTEM) {
        return libdnf5::OPTIONAL_METADATA_TYPES;
    } else {
        return config.get_main_config().get_optional_metadata_types_option().get_value();
    }
}

}  // namespace libdnf5::repo
