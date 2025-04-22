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

#ifndef LIBDNF5_REPO_REPO_CACHE_HPP
#define LIBDNF5_REPO_REPO_CACHE_HPP

#include "repo_cache_errors.hpp"

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/defs.h"

#include <filesystem>
#include <string>


namespace libdnf5::repo {


struct LIBDNF_API RepoCacheRemoveStatistics {
    RepoCacheRemoveStatistics();
    ~RepoCacheRemoveStatistics();

    RepoCacheRemoveStatistics(const RepoCacheRemoveStatistics & src);
    RepoCacheRemoveStatistics & operator=(const RepoCacheRemoveStatistics & src);

    RepoCacheRemoveStatistics(RepoCacheRemoveStatistics && src) noexcept;
    RepoCacheRemoveStatistics & operator=(RepoCacheRemoveStatistics && src) noexcept;

    std::size_t get_files_removed();     // Number of removed files and links.
    std::size_t get_dirs_removed();      // Number of removed directorires.
    std::uintmax_t get_bytes_removed();  // Number of removed bytes.
    std::size_t get_errors();            // Numbes of errors.

    RepoCacheRemoveStatistics & operator+=(const RepoCacheRemoveStatistics & rhs) noexcept;

private:
    friend class RepoCache;

    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};


/// Repository cache management class.
class LIBDNF_API RepoCache {
public:
    using RemoveStatistics = RepoCacheRemoveStatistics;

    /// The name of the attribute used to mark the cache as expired.
    static constexpr const char * ATTRIBUTE_EXPIRED = "expired";

    /// Construct a new repository cache management instance.
    ///
    /// @param base            WeakPtr on the Base instance.
    /// @param repo_cache_dir  Path to repository cache directory.
    RepoCache(const libdnf5::BaseWeakPtr & base, const std::filesystem::path & repo_cache_dir);

    ~RepoCache();

    RepoCache(const RepoCache & src);
    RepoCache & operator=(const RepoCache & src);

    RepoCache(RepoCache && src) noexcept;
    RepoCache & operator=(RepoCache && src) noexcept;

    /// Construct a new repository cache management instance.
    ///
    /// @param base            Base instance reference.
    /// @param repo_cache_dir  Path to repository cache directory.
    RepoCache(libdnf5::Base & base, const std::string & repo_cache_dir);

    /// Removes metadata from the cache.
    ///
    /// @return Number of deleted files and directories. Number of errors.
    RemoveStatistics remove_metadata();

    /// Removes packages from the cache.
    ///
    /// @return Number of deleted files and directories. Number of errors.
    RemoveStatistics remove_packages();

    /// Removes solvable files from the cache.
    ///
    /// @return Number of deleted files and directories. Number of errors.
    RemoveStatistics remove_solv_files();

    /// Removes metadata, packages, solvable files, and attributes from the cache.
    /// If the repository cache directory becomes empty, it will also be deleted.
    ///
    /// @return Number of deleted files and directories. Number of errors.
    RemoveStatistics remove_all();

    /// Writes `value` to the` name` attribute.
    /// If the attribute does not exist, it is created. If it already existed, its value is overwritten.
    ///
    /// @param name   Attribute name.
    /// @param value  Attribute value.
    /// @exception std::runtime_exception  An error occurred while writing the attribute.
    void write_attribute(const std::string & name, const std::string & value = "");

    /// Returns the value of the `name` attribute.
    ///
    /// @param name  Attribute name.
    /// @return The value of the `name` attribute.
    /// @exception std::runtime_exception  The `name` attribute does not exist or an error occurred while reading it.
    std::string read_attribute(const std::string & name);

    /// Tests if the `name` attribute exists.
    ///
    /// @param name  Attribute name.
    /// @return If the `name` attribute exists, it returns true otherwise false.
    bool is_attribute(const std::string & name);

    /// Removes the `name` attribute.
    ///
    /// @param name  Attribute name.
    /// @return true if the attribute was deleted, false if the attribute did not exist.
    /// @exception std::runtime_exception An error occurred while removing the attribute.
    bool remove_attribute(const std::string & name);

    /// Gets the id of the cached repository.
    ///
    /// @return id of the cached repository.
    /// @exception RepoCacheException Throws an exception if the reposiitory id cannot be determined.
    std::string get_repoid();

private:
    friend RepoCacheRemoveStatistics;

    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};


}  // namespace libdnf5::repo

#endif
