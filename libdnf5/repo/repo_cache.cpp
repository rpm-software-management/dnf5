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


#include "repo_cache_private.hpp"
#include "utils/bgettext/bgettext-mark-domain.h"
#include "utils/fs/file.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/logger/logger.hpp"


namespace libdnf5::repo {


namespace {

constexpr const char * CACHE_ATTRS_DIR = "attrs";
constexpr const char * CACHE_MIRRORLIST_FILE = "mirrorlist";
constexpr const char * CACHE_METALINK_FILE = "metalink.xml";


// Removes a file or empty directory.
// In case of an error, the error_count increases by one.
// Returns number of removed items (0 or 1).
std::size_t remove(const std::filesystem::path & path, std::size_t & error_count, Logger & log) noexcept {
    try {
        if (std::filesystem::remove(path)) {
            return 1;
        }
    } catch (const std::filesystem::filesystem_error & ex) {
        ++error_count;
        log.warning("Cannot remove \"{}\": {}", path.native(), ex.what());
    }
    return 0;
}


// Removes directory and its content.
RepoCache::RemoveStatistics remove_recursive(const std::filesystem::path & dir_path, Logger & log) {
    RepoCache::RemoveStatistics status{};
    std::error_code ec;
    for (const auto & dir_entry : std::filesystem::directory_iterator(dir_path, ec)) {
        if (dir_entry.is_directory()) {
            status += remove_recursive(dir_entry, log);
            status.dirs_removed += remove(dir_entry, status.errors, log);
        } else {
            status.files_removed += remove(dir_entry, status.errors, log);
        }
    }
    status.dirs_removed += remove(dir_path, status.errors, log);
    return status;
}


inline std::filesystem::path get_attribute_filepath(
    const std::filesystem::path & repo_cache_dir, const std::string & attr_name) {
    return repo_cache_dir / CACHE_ATTRS_DIR / attr_name;
}


/// Removes all attributes from the cache.
RepoCache::RemoveStatistics cache_remove_attributes(const std::filesystem::path & path, Logger & log) {
    auto status = remove_recursive(path / CACHE_ATTRS_DIR, log);
    log.debug(
        "Attributes removal from repository cache in path \"{}\" complete. Removed {} files, {} directories. {} errors",
        path.native(),
        status.files_removed,
        status.dirs_removed,
        status.errors);
    return status;
}

}  // namespace


RepoCacheRemoveStatistics & RepoCacheRemoveStatistics::operator+=(const RepoCacheRemoveStatistics & rhs) noexcept {
    files_removed += rhs.files_removed;
    dirs_removed += rhs.dirs_removed;
    errors += rhs.errors;
    return *this;
}


RepoCache::RepoCache(const libdnf5::BaseWeakPtr & base, const std::filesystem::path & repo_cache_dir)
    : base(base),
      cache_dir(repo_cache_dir) {
    if (cache_dir.empty()) {
        throw RepoCacheError(M_("Empty path to the repository cache directory."));
    }
}


RepoCache::RepoCache(libdnf5::Base & base, const std::string & repo_cache_dir)
    : RepoCache(base.get_weak_ptr(), repo_cache_dir) {}


RepoCache::RemoveStatistics RepoCache::remove_metadata() {
    auto & log = *base->get_logger();
    auto status = remove_recursive(cache_dir / CACHE_METADATA_DIR, log);

    status.files_removed += remove(cache_dir / CACHE_MIRRORLIST_FILE, status.errors, log);
    status.files_removed += remove(cache_dir / CACHE_METALINK_FILE, status.errors, log);
    log.debug(
        "Metadata removal from repository cache in path \"{}\" complete. Removed {} files, {} directories. {} errors",
        cache_dir.native(),
        status.files_removed,
        status.dirs_removed,
        status.errors);
    return status;
}


RepoCache::RemoveStatistics RepoCache::remove_packages() {
    auto & log = *base->get_logger();
    auto status = remove_recursive(cache_dir / CACHE_PACKAGES_DIR, log);
    log.debug(
        "Packages removal from repository cache in path \"{}\" complete. Removed {} files, {} directories. {} errors",
        cache_dir.native(),
        status.files_removed,
        status.dirs_removed,
        status.errors);
    return status;
}


RepoCache::RemoveStatistics RepoCache::remove_solv_files() {
    auto & log = *base->get_logger();
    auto status = remove_recursive(cache_dir / CACHE_SOLV_FILES_DIR, log);
    log.debug(
        "Solv files removal from repository cache in path \"{}\" complete. Removed {} files, {} directories. {} errors",
        cache_dir.native(),
        status.files_removed,
        status.dirs_removed,
        status.errors);
    return status;
}


RepoCache::RemoveStatistics RepoCache::remove_all() {
    auto & log = *base->get_logger();
    auto status = remove_metadata();
    status += remove_packages();
    status += remove_solv_files();
    status += cache_remove_attributes(cache_dir, log);
    std::error_code ec;
    if (std::filesystem::remove(cache_dir, ec)) {
        ++status.dirs_removed;
    }
    return status;
}


void RepoCache::write_attribute(const std::string & name, const std::string & value) {
    std::filesystem::create_directory(std::filesystem::path(cache_dir) / CACHE_ATTRS_DIR);
    utils::fs::File attr_file(get_attribute_filepath(cache_dir, name), "w", false);
    attr_file.write(value);
    attr_file.close();  // unlike a destructor, it can throw an exception
}


std::string RepoCache::read_attribute(const std::string & name) {
    utils::fs::File attr_file(get_attribute_filepath(cache_dir, name), "r", false);
    return attr_file.read();
}


bool RepoCache::is_attribute(const std::string & name) {
    return std::filesystem::exists(get_attribute_filepath(cache_dir, name));
}


bool RepoCache::remove_attribute(const std::string & name) {
    return std::filesystem::remove(get_attribute_filepath(cache_dir, name));
}


std::string RepoCache::get_repoid() {
    constexpr std::size_t HASH_LENGTH = 16;
    constexpr std::size_t HYPHEN_POS_FROM_END = HASH_LENGTH + 1;
    constexpr const char * HASH_CHARS = "0123456789ABCDEFabcdef";

    auto it = --cache_dir.end();
    if (it->empty()) {
        --it;
    }
    std::string name(*it);
    if (name.length() > HYPHEN_POS_FROM_END && name[name.length() - HYPHEN_POS_FROM_END] == '-') {
        if (name.find_first_not_of(HASH_CHARS, name.length() - HASH_LENGTH) == std::string::npos) {
            return name.substr(0, name.length() - HYPHEN_POS_FROM_END);
        }
    }
    throw RepoCacheError(M_("Unable to determine id of the repository in the cache."));
}


}  // namespace libdnf5::repo
