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

#include "libdnf5/base/base.hpp"
#include "libdnf5/logger/logger.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"
#include "libdnf5/utils/fs/file.hpp"


namespace libdnf5::repo {


namespace {

constexpr const char * CACHE_ATTRS_DIR = "attrs";


// Removes a file or empty directory.
// In case of an error, the error_count increases by one.
// Returns number of removed items (0 or 1).
std::size_t remove(
    const std::filesystem::path & path,
    std::size_t & error_count,
    std::uintmax_t & bytes_count,
    Logger & log) noexcept {
    try {
        std::error_code ec;
        auto size = std::filesystem::file_size(path, ec);
        if (ec.value() == 21) {
            // `path` is a directory. Allocated space is FS depended (usually 4096 bytes). Do not risk it
            size = 0;
        } else if (ec) {
            // Usually error #2 (No such file or directory)
            size = 0;
        }
        if (std::filesystem::remove(path)) {
            bytes_count += size;
            return 1;
        }
    } catch (const std::filesystem::filesystem_error & ex) {
        ++error_count;
        log.warning("Cannot remove \"{}\": {}", path.native(), ex.what());
    }
    return 0;
}

inline std::filesystem::path get_attribute_filepath(
    const std::filesystem::path & repo_cache_dir, const std::string & attr_name) {
    return repo_cache_dir / CACHE_ATTRS_DIR / attr_name;
}


}  // namespace


/// Removes all attributes from the cache.
RepoCache::RemoveStatistics RepoCache::Impl::cache_remove_attributes(const std::filesystem::path & path, Logger & log) {
    auto status = remove_recursive(path / CACHE_ATTRS_DIR, log);
    log.debug(
        "Attributes removal from repository cache in path \"{}\" complete. "
        "Removed {} files, {} directories (total of {} bytes). {} errors",
        path.native(),
        status.get_files_removed(),
        status.get_dirs_removed(),
        status.get_bytes_removed(),  // Print actual bytes (instead of MiB, GiB, ...) for debug
        status.get_errors());
    return status;
}

// Removes directory and its content.
RepoCache::RemoveStatistics RepoCache::Impl::remove_recursive(const std::filesystem::path & dir_path, Logger & log) {
    RepoCache::RemoveStatistics status{};
    std::error_code ec;
    for (const auto & dir_entry : std::filesystem::directory_iterator(dir_path, ec)) {
        if (dir_entry.is_directory() && !dir_entry.is_symlink()) {
            status += remove_recursive(dir_entry, log);
            status.p_impl->dirs_removed += remove(dir_entry, status.p_impl->errors, status.p_impl->bytes_removed, log);
        } else {
            status.p_impl->files_removed += remove(dir_entry, status.p_impl->errors, status.p_impl->bytes_removed, log);
        }
    }
    status.p_impl->dirs_removed += remove(dir_path, status.p_impl->errors, status.p_impl->bytes_removed, log);
    return status;
}


RepoCacheRemoveStatistics & RepoCacheRemoveStatistics::operator+=(const RepoCacheRemoveStatistics & rhs) noexcept {
    p_impl->files_removed += rhs.p_impl->files_removed;
    p_impl->dirs_removed += rhs.p_impl->dirs_removed;
    p_impl->bytes_removed += rhs.p_impl->bytes_removed;
    p_impl->errors += rhs.p_impl->errors;
    return *this;
}

RepoCacheRemoveStatistics::RepoCacheRemoveStatistics() : p_impl(new Impl()) {};
RepoCacheRemoveStatistics::~RepoCacheRemoveStatistics() = default;

RepoCacheRemoveStatistics::RepoCacheRemoveStatistics(const RepoCacheRemoveStatistics & src) = default;
RepoCacheRemoveStatistics::RepoCacheRemoveStatistics(RepoCacheRemoveStatistics && src) noexcept = default;

RepoCacheRemoveStatistics & RepoCacheRemoveStatistics::operator=(const RepoCacheRemoveStatistics & src) = default;
RepoCacheRemoveStatistics & RepoCacheRemoveStatistics::operator=(RepoCacheRemoveStatistics && src) noexcept = default;

std::size_t RepoCacheRemoveStatistics::get_files_removed() {
    return p_impl->files_removed;
}
std::size_t RepoCacheRemoveStatistics::get_dirs_removed() {
    return p_impl->dirs_removed;
}
std::uintmax_t RepoCacheRemoveStatistics::get_bytes_removed() {
    return p_impl->bytes_removed;
}
std::size_t RepoCacheRemoveStatistics::get_errors() {
    return p_impl->errors;
}

RepoCache::RepoCache(const libdnf5::BaseWeakPtr & base, const std::filesystem::path & repo_cache_dir)
    : p_impl(new Impl(base, repo_cache_dir)) {}


RepoCache::RepoCache(libdnf5::Base & base, const std::string & repo_cache_dir)
    : RepoCache(base.get_weak_ptr(), repo_cache_dir) {}


RepoCache::~RepoCache() = default;

RepoCache::RepoCache(const RepoCache & src) = default;
RepoCache::RepoCache(RepoCache && src) noexcept = default;

RepoCache & RepoCache::operator=(const RepoCache & src) = default;
RepoCache & RepoCache::operator=(RepoCache && src) noexcept = default;

RepoCache::RemoveStatistics RepoCache::remove_metadata() {
    auto & log = *p_impl->base->get_logger();
    auto status = p_impl->remove_recursive(p_impl->cache_dir / CACHE_METADATA_DIR, log);

    status.p_impl->files_removed +=
        remove(p_impl->cache_dir / CACHE_MIRRORLIST_FILE, status.p_impl->errors, status.p_impl->bytes_removed, log);
    status.p_impl->files_removed +=
        remove(p_impl->cache_dir / CACHE_METALINK_FILE, status.p_impl->errors, status.p_impl->bytes_removed, log);
    log.debug(
        "Metadata removal from repository cache in path \"{}\" complete. "
        "Removed {} files, {} directories (total of {} bytes). {} errors",
        p_impl->cache_dir.native(),
        status.get_files_removed(),
        status.get_dirs_removed(),
        status.get_bytes_removed(),
        status.get_errors());
    return status;
}


RepoCache::RemoveStatistics RepoCache::remove_packages() {
    auto & log = *p_impl->base->get_logger();
    auto status = p_impl->remove_recursive(p_impl->cache_dir / CACHE_PACKAGES_DIR, log);
    log.debug(
        "Packages removal from repository cache in path \"{}\" complete. "
        "Removed {} files, {} directories (total of {} bytes). {} errors",
        p_impl->cache_dir.native(),
        status.get_files_removed(),
        status.get_dirs_removed(),
        status.get_bytes_removed(),
        status.get_errors());
    return status;
}


RepoCache::RemoveStatistics RepoCache::remove_solv_files() {
    auto & log = *p_impl->base->get_logger();
    auto status = p_impl->remove_recursive(p_impl->cache_dir / CACHE_SOLV_FILES_DIR, log);
    log.debug(
        "Solv files removal from repository cache in path \"{}\" complete. "
        "Removed {} files, {} directories (total of {} bytes). {} errors",
        p_impl->cache_dir.native(),
        status.get_files_removed(),
        status.get_dirs_removed(),
        status.get_bytes_removed(),
        status.get_errors());
    return status;
}


RepoCache::RemoveStatistics RepoCache::remove_all() {
    auto & log = *p_impl->base->get_logger();
    auto status = remove_metadata();
    status += remove_packages();
    status += remove_solv_files();
    status += p_impl->cache_remove_attributes(p_impl->cache_dir, log);
    std::error_code ec;
    if (std::filesystem::remove(p_impl->cache_dir, ec)) {
        ++status.p_impl->dirs_removed;
    }
    return status;
}


void RepoCache::write_attribute(const std::string & name, const std::string & value) {
    std::filesystem::create_directory(std::filesystem::path(p_impl->cache_dir) / CACHE_ATTRS_DIR);
    utils::fs::File attr_file(get_attribute_filepath(p_impl->cache_dir, name), "w", false);
    attr_file.write(value);
    attr_file.close();  // unlike a destructor, it can throw an exception
}


std::string RepoCache::read_attribute(const std::string & name) {
    utils::fs::File attr_file(get_attribute_filepath(p_impl->cache_dir, name), "r", false);
    return attr_file.read();
}


bool RepoCache::is_attribute(const std::string & name) {
    return std::filesystem::exists(get_attribute_filepath(p_impl->cache_dir, name));
}


bool RepoCache::remove_attribute(const std::string & name) {
    return std::filesystem::remove(get_attribute_filepath(p_impl->cache_dir, name));
}


std::string RepoCache::get_repoid() {
    constexpr std::size_t HASH_LENGTH = 16;
    constexpr std::size_t HYPHEN_POS_FROM_END = HASH_LENGTH + 1;
    constexpr const char * HASH_CHARS = "0123456789ABCDEFabcdef";

    auto it = --p_impl->cache_dir.end();
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
