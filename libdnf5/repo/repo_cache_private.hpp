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

#ifndef LIBDNF5_REPO_REPO_CACHE_PRIVATE_HPP
#define LIBDNF5_REPO_REPO_CACHE_PRIVATE_HPP

#include "libdnf5/logger/logger.hpp"
#include "libdnf5/repo/repo_cache.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"


namespace libdnf5::repo {


namespace {

constexpr const char * CACHE_METADATA_DIR = "repodata";
constexpr const char * CACHE_PACKAGES_DIR = "packages";
constexpr const char * CACHE_SOLV_FILES_DIR = "solv";
constexpr const char * CACHE_METALINK_FILE = "metalink.xml";
constexpr const char * CACHE_MIRRORLIST_FILE = "mirrorlist";

}  // namespace


class RepoCacheRemoveStatistics::Impl {
private:
    friend RepoCacheRemoveStatistics;
    friend RepoCache;

    std::size_t files_removed;     // Number of removed files and links.
    std::size_t dirs_removed;      // Number of removed directorires.
    std::uintmax_t bytes_removed;  // Number of removed bytes.
    std::size_t errors;            // Numbes of errors.
};

class RepoCache::Impl {
public:
    Impl(const libdnf5::BaseWeakPtr & base, std::filesystem::path repo_cache_dir)
        : base(base),
          cache_dir(std::move(repo_cache_dir)) {
        if (cache_dir.empty()) {
            throw RepoCacheError(M_("Empty path to the repository cache directory."));
        }
    }

private:
    friend RepoCache;

    RepoCache::RemoveStatistics remove_recursive(const std::filesystem::path & dir_path, Logger & log);
    RepoCache::RemoveStatistics cache_remove_attributes(const std::filesystem::path & path, Logger & log);
    libdnf5::BaseWeakPtr base;
    std::filesystem::path cache_dir;
};

}  // namespace libdnf5::repo

#endif
