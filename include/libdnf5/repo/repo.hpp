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

#ifndef LIBDNF5_RPM_REPO_HPP
#define LIBDNF5_RPM_REPO_HPP

#define MODULEMD

#include "config_repo.hpp"
#include "repo_callbacks.hpp"

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/common/exception.hpp"
#include "libdnf5/common/weak_ptr.hpp"
#include "libdnf5/defs.h"
#include "libdnf5/repo/repo_errors.hpp"
#include "libdnf5/repo/repo_weak.hpp"
#include "libdnf5/rpm/package.hpp"

#include <memory>


namespace libdnf5::solv {
class Pool;
}


namespace libdnf5::rpm {
class Package;
class PackageSack;
}  // namespace libdnf5::rpm


namespace libdnf5::repo {

class SolvRepo;
class RepoDownloader;


/// RPM repository
/// Represents a repository used to download packages.
/// Remote metadata is cached locally.
class LIBDNF_API Repo {
public:
    enum class Type { AVAILABLE, SYSTEM, COMMANDLINE };

    enum class SyncStrategy {
        // use the local cache even if it's expired. download if there's no cache.
        LAZY = 1,
        // use the local cache, even if it's expired, never download.
        ONLY_CACHE = 2,
        // try the cache, if it is expired download new md.
        TRY_CACHE = 3
    };

    /// Verify repo ID
    /// @param repo_id repo ID to verify
    /// @return   index of the first invalid character in the repo ID (if present) or std::string::npos
    // @replaces libdnf:repo/Repo.hpp:method:Repo.verifyId(const std::string & id)
    static std::string::size_type verify_id(const std::string & repo_id);

    /// Construct the Repo object
    /// @param base   weak pointer to the Base instance
    /// @param id     repo ID to use
    /// @param type   type of repo
    Repo(const libdnf5::BaseWeakPtr & base, const std::string & id, Repo::Type type = Repo::Type::AVAILABLE);

    /// Construct the Repo object
    /// @param base   a reference to the Base instance
    /// @param id     repo ID to use
    /// @param type   type of repo
    Repo(libdnf5::Base & base, const std::string & id, Repo::Type type = Repo::Type::AVAILABLE);

    ~Repo();

    /// Returns the repository type
    Type get_type() const noexcept;

    /// Registers a class that implements callback methods (fastest mirror detection, download state, key import).
    // @replaces libdnf:repo/Repo.hpp:method:Repo.setCallbacks(std::unique_ptr<RepoCallbacks> && callbacks)
    void set_callbacks(std::unique_ptr<libdnf5::repo::RepoCallbacks> && callbacks);

    /// Returns the currently registered callbacks for the repo.
    std::unique_ptr<libdnf5::repo::RepoCallbacks> & get_callbacks();

    /// @brief Sets the associated user data. These are used in callbacks.
    /// @param user_data  Pointer to user data
    void set_user_data(void * user_data) noexcept;

    /// @brief Gets the associated user data.
    /// @return Pointer to user data
    void * get_user_data() const noexcept;

    /// Verify repo object configuration
    /// Will throw exception if Repo has no mirror or baseurl set or if Repo type is unsupported.
    // @replaces libdnf:repo/Repo.hpp:method:Repo.verify()
    void verify() const;

    /// Returns pointer to the repository configuration
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getConfig()
    ConfigRepo & get_config() noexcept;

    /// Returns pointer to the repository configuration
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getConfig()
    const ConfigRepo & get_config() const noexcept;

    /// Returns repository id
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getId()
    std::string get_id() const noexcept;

    /// Enable the repository
    // @replaces libdnf:repo/Repo.hpp:method:Repo.enable()
    void enable();

    /// Disable the repository
    // @replaces libdnf:repo/Repo.hpp:method:Repo.disable()
    void disable();

    /// Return whether the repository is enabled.
    /// @return true if enabled
    // @replaces libdnf:repo/Repo.hpp:method:Repo.isEnabled()
    bool is_enabled() const;

    /// Return whether the repository is local.
    /// @return true if local
    // @replaces libdnf:repo/Repo.hpp:method:Repo.isLocal()
    bool is_local() const;

    /// Reads metadata from local cache.
    // @replaces libdnf:repo/Repo.hpp:method:Repo.loadCache(bool throwExcept)
    void read_metadata_cache();

    /// Checks whether the locally downloaded metadata are in sync with the origin.
    /// @return `true` if metadata are in sync with the origin, `false` otherwise.
    bool is_in_sync();

    /// Returns whether the using of "includes" is enabled
    /// If enabled, only packages listed in the "includepkgs" will be used from the repository.
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getUseIncludes()
    bool get_use_includes() const;

    /// Enables/disables using of "includes"
    /// If enabled, only packages listed in the "includepkgs" will be used from the repository.
    // @replaces libdnf:repo/Repo.hpp:method:Repo.setUseIncludes(bool enabled)
    void set_use_includes(bool enabled);

    /// Returns repository cost
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getCost()
    /// TODO(jrohel): Remove it? It is only shortcut for get_config()->cost()->get_value()
    int get_cost() const;

    /// @brief Set repo cost in RepoConf and in Libsolv repo if attached. Values are only updated when Option::Priority
    /// of stored value is equal or lower.
    ///
    /// @version 1.0.0
    /// @param value Cost value
    /// @param priority Optional argument
    void set_cost(int value, Option::Priority priority = Option::Priority::RUNTIME);

    /// Returns repository priority
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getPriority()
    /// TODO(jrohel): Remove it? It is only shortcut for get_config()->cost()->get_value()
    int get_priority() const;

    /// @brief Set repo priority in RepoConf and in Libsolv repo if attached. Values are only updated when
    /// Option::Priority of stored value is equal or lower.
    ///
    /// @version 1.0.0
    /// @param value Priority value
    /// @param priority Optional argument
    void set_priority(int value, Option::Priority priority = Option::Priority::RUNTIME);

    // @replaces libdnf:repo/Repo.hpp:method:Repo.getRevision()
    const std::string & get_revision() const;

    /// Gets age of the local cache
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getAge()
    int64_t get_age() const;

    /// Return path to the particular downloaded repository metadata in cache
    /// @param metadata_type metadata type (filelists, other, productid...)
    /// @return file path or empty string in case the requested metadata does not exist
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getMetadataPath(const std::string & metadataType)
    std::string get_metadata_path(const std::string & metadata_type);

    /// Mark whatever is in the current cache expired.
    /// This repo instance will always try to fetch a fresh metadata after this
    /// method is called.
    // @replaces libdnf:repo/Repo.hpp:method:Repo.expire()
    void expire();

    /// @brief Return whether the cached metadata is expired.
    /// @return bool
    // @replaces libdnf:repo/Repo.hpp:method:Repo.isExpired()
    bool is_expired() const;

    /// @brief Get the number of seconds after which the cached metadata will expire.
    /// Negative number means the metadata has expired already.
    /// @return Seconds to expiration
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getExpiresIn()
    int get_expires_in() const;

    // @replaces libdnf:repo/Repo.hpp:method:Repo.setMaxMirrorTries(int maxMirrorTries)
    void set_max_mirror_tries(int max_mirror_tries);

    /// Gets timestamp of metadata "primary" file, if the file is not present returns -1
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getTimestamp()
    int64_t get_timestamp() const;

    /// Gets the highest timestamp from repomd records
    /// TODO(jrohel): Used in DNF repolist: displayed as "Repo-updated time" base.py: "using metadata from" in debug messages Is it correct?
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getMaxTimestamp()
    int get_max_timestamp();

    /// Try to preserve remote side timestamps
    /// When set to true the underlying librepo is asked to make an attempt to set the timestamps
    /// of the local downloaded files (repository metadata and packages) to match those from
    /// the remote files.
    /// This feature is by default switched off.
    /// @param preserve_remote_time true - use remote file timestamp, false - use the current time
    // @replaces libdnf:repo/Repo.hpp:method:Repo.setPreserveRemoteTime(bool preserveRemoteTime)
    void set_preserve_remote_time(bool preserve_remote_time);

    // @replaces libdnf:repo/Repo.hpp:method:Repo.getPreserveRemoteTime()
    bool get_preserve_remote_time() const;

    /// TODO(jrohel): Used by DNF repolist. Do we need it?
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getContentTags()
    const std::vector<std::string> & get_content_tags();

    /// TODO(jrohel): Used by DNF repolist. Do we need it?
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getDistroTags()
    const std::vector<std::pair<std::string, std::string>> & get_distro_tags();

    /// Get list of relative locations of metadata files inside the repo
    /// e.g. [('primary', 'repodata/primary.xml.gz'), ('filelists', 'repodata/filelists.xml.gz')...]
    /// @return vector of (metadata_type, location) string pairs
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getMetadataLocations()
    const std::vector<std::pair<std::string, std::string>> get_metadata_locations() const;

    /// Gets path to the repository cache directory
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getCacheDir()
    std::string get_cachedir() const;

    /// Gets path to the repository persistent directory
    std::string get_persistdir() const;

    /// Gets name of repository
    /// Alias
    std::string get_name() { return this->get_config().get_name_option().get_value(); };
    /// Sets repository configuration file path
    // @replaces libdnf:repo/Repo.hpp:method:Repo.setRepoFilePath(const std::string & path)
    void set_repo_file_path(const std::string & path);

    /// Gets repository configuration file path
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getRepoFilePath()
    const std::string & get_repo_file_path() const noexcept;

    /// Sets repository synchronisation strategy
    // @replaces libdnf:repo/Repo.hpp:method:Repo.setSyncStrategy(SyncStrategy strategy)
    void set_sync_strategy(SyncStrategy strategy);

    /// Returns repository synchronisation strategy
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getSyncStrategy()
    SyncStrategy get_sync_strategy() const noexcept;

    /// Downloads file from URL into given opened file descriptor.
    // @replaces libdnf:repo/Repo.hpp:method:Repo.downloadUrl(const char * url, int fd)
    /// TODO(lukash) fd seems like an inconvenient API for this function, use target path instead?
    ///              It also needs defining what it means downloading an URL through a particular repo
    //void download_url(const char * url, int fd);

    /// Set http headers.
    /// @param headers A vector of full headers ("header: value")
    // @replaces libdnf:repo/Repo.hpp:method:Repo.setHttpHeaders(const char * headers[])
    void set_http_headers(const std::vector<std::string> & headers);

    /// Get http headers.
    /// @return A vector of full headers ("header: value")
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getHttpHeaders()
    std::vector<std::string> get_http_headers() const;

    /// Returns mirrorlist associated with the repository.
    /// Mirrors on this list are mirrors parsed from mirrorlist/metalink specified by LRO_MIRRORLIST or
    /// from mirrorlist specified by LRO_MIRROSLISTURL and metalink specified by LRO_METALINKURL.
    /// No URLs specified by LRO_URLS are included in this list.
    // @replaces libdnf:repo/Repo.hpp:method:Repo.getMirrors()
    std::vector<std::string> get_mirrors() const;

    libdnf5::repo::RepoWeakPtr get_weak_ptr();

    /// @return The `Base` object to which this object belongs.
    /// @since 5.0
    libdnf5::BaseWeakPtr get_base() const;

    /// Return string representation of the Type enum
    static std::string type_to_string(Type type);

private:
    class LIBDNF_LOCAL Impl;
    friend class RepoSack;
    friend class rpm::Package;
    friend class rpm::PackageSack;
    friend class FileDownloader;
    friend class PackageDownloader;
    friend class solv::Pool;

    /// Loads the repository objects into sacks.
    ///
    /// Also writes the libsolv's solv/solvx cache files.
    LIBDNF_LOCAL void load();

    /// Downloads repository metadata.
    // @replaces libdnf:repo/Repo.hpp:method:Repo.downloadMetadata(const std::string & destdir)
    LIBDNF_LOCAL void download_metadata(const std::string & destdir);

    LIBDNF_LOCAL void add_libsolv_testcase(const std::string & path);

    /// Adds an RPM package at `path` to the repository.
    ///
    /// If `with_hdrid` is `true`, the RPM is loaded with the
    /// `RPM_ADD_WITH_HDRID | RPM_ADD_WITH_SHA256SUM` flags, meaning libsolv will
    /// calculate the SHA256 checksum of the RPM header and store it. This adds
    /// overhead but has the advantage of TODO(lukash) describe the advantage.
    /// @param path The path to the RPM file.
    /// @param with_hdrid If true, libsolv calculates header checksum and stores it.
    /// @throws RepoRpmError if the RPM file can't be read or is corrupted.
    /// @return PackageId of the added package.
    LIBDNF_LOCAL libdnf5::rpm::Package add_rpm_package(const std::string & path, bool with_hdrid);

    LIBDNF_LOCAL void make_solv_repo();

    LIBDNF_LOCAL void load_available_repo();
    LIBDNF_LOCAL void load_system_repo();

    LIBDNF_LOCAL void internalize();

    /// If the repository is not already marked as expired, it checks for the presence of the repository cache
    /// expiration attribute, and if the metadata_expire configuration value is set, also checks the modification times
    /// of the main configuration file, the repository configuration file, and the cached primary file.
    /// Depending on the result, the repository may be marked as expired.
    LIBDNF_LOCAL void recompute_expired();

    /// @brief  Clones repodata and solv files from the root cache. The original user repository cache is deleted.
    ///         The intended use case is for cloning the root cache when the user one is invalid or empty.
    /// @return Whether at least the repodata cache cloning was successful.
    LIBDNF_LOCAL bool clone_root_metadata();

    LIBDNF_LOCAL RepoDownloader & get_downloader() const;

    LIBDNF_LOCAL bool is_loaded() const;

    /// Requires that the repo is loaded
    LIBDNF_LOCAL SolvRepo & get_solv_repo() const;

    /// Mark this repository as fresh (it is not expired).
    LIBDNF_LOCAL void mark_fresh();

    // Add xml comps file at `path` to the repository.
    LIBDNF_LOCAL void add_xml_comps(const std::string & path);

    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::repo

#endif
