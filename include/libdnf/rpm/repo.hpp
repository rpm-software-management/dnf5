/*
Copyright (C) 2018-2021 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_RPM_REPO_HPP
#define LIBDNF_RPM_REPO_HPP

#define MODULEMD

#include "config_repo.hpp"

#include "libdnf/utils/exception.hpp"

#include <memory>

namespace libdnf {

class Base;

}  // namespace libdnf

namespace libdnf::rpm {

class SolvSack;

class LrException : public RuntimeError {
public:
    LrException(int code, const char * msg) : RuntimeError(msg), code(code) {}
    LrException(int code, const std::string & msg) : RuntimeError(msg), code(code) {}
    const char * get_domain_name() const noexcept override { return "librepo"; }
    const char * get_name() const noexcept override { return "LrException"; }
    const char * get_description() const noexcept override { return "Librepo exception"; }
    int get_code() const noexcept { return code; }

private:
    int code;
};

/// Base class for Repo callbacks
/// User implements repo callbacks by inheriting this class and overriding its methods.
class RepoCB {
public:
    enum class FastestMirrorStage {
        /// Fastest mirror detection just started. ptr is NULL
        INIT,

        /// ptr is (char *) pointer to string with path to the cache file. Do not modify or free the string.
        CACHELOADING,

        /// if cache was loaded successfully, ptr is NULL, otherwise ptr is (char *) string with error message.
        /// Do not modify or free the string.
        CACHELOADINGSTATUS,

        /// Detection (pinging) in progress.
        /// If all data was loaded from cache, this stage is skiped. ptr is pointer to long.
        /// This is the number of how much mirrors have to be "pinged"
        DETECTION,

        /// Detection is done, sorting mirrors, updating cache, etc. ptr is NULL
        FINISHING,

        /// The very last invocation of fastest mirror callback.
        /// If fastest mirror detection was successful ptr is NULL, otherwise ptr contain (char *) string with error message.
        /// Do not modify or free the string.
        STATUS,
    };

    virtual void start(const char * what);
    virtual void end() {}
    virtual int progress(double total_to_download, double downloaded);
    virtual void fastest_mirror(FastestMirrorStage stage, const char * msg);
    virtual int handle_mirror_failure(const char * msg, const char * url, const char * metadata);
    virtual bool repokey_import(
        const std::string & id,
        const std::string & user_id,
        const std::string & fingerprint,
        const std::string & url,
        long int timestamp);
    virtual ~RepoCB() = default;
};

/// RPM repository
/// Represents a repository used to download packages.
/// Remote metadata is cached locally.
class Repo {
public:
    // TODO(jrohel): Do we need this Repo class for SYSTEM and COMMANDINE repos? Thinking about removing enum.
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
    /// @param id repo ID to verify
    /// @return   index of the first invalid character in the repo ID (if present) or std::string::npos
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.verifyId(const std::string & id)
    static std::string::size_type verify_id(const std::string & repo_id);

    /// Construct the Repo object
    /// @param id     repo ID to use
    /// @param conf   configuration to use
    /// @param base   reference to Base instance
    /// @param type   type of repo
    Repo(const std::string & id, Base & base, Repo::Type type = Repo::Type::AVAILABLE);

    Repo & operator=(Repo && repo) = delete;

    /// Registers a class that implements callback methods (fastest mirror detection, download state, key import).
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.setCallbacks(std::unique_ptr<RepoCB> && callbacks)
    void set_callbacks(std::unique_ptr<RepoCB> && callbacks);

    /// Verify repo object configuration
    /// Will throw exception if Repo has no mirror or baseurl set or if Repo type is unsupported.
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.verify()
    void verify() const;

    /// Returns pointer to the repository configuration
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getConfig()
    ConfigRepo & get_config() noexcept;

    /// Returns repository id
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getId()
    const std::string & get_id() const noexcept;

    /// Enable the repository
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.enable()
    void enable();

    /// Disable the repository
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.disable()
    void disable();

    /// Return whether the repository is enabled.
    /// @return true if enabled
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.isEnabled()
    bool is_enabled() const;

    /// Return whether the repository is local.
    /// @return true if local
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.isLocal()
    bool is_local() const;

    /// Initialize the repo with metadata
    /// Fetches new metadata from the origin or just reuses local cache if still valid.
    /// @return true if fresh metadata were downloaded, false otherwise.
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.load()
    bool load();

    /// Loads metadata from local cache
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.loadCache(bool throwExcept)
    void load_cache();

    /// Downloads all metadata
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.downloadMetadata(const std::string & destdir)
    void download_metadata(const std::string & destdir);

    /// Returns whether the using of "includes" is enabled
    /// If enabled, only packages listed in the "includepkgs" will be used from the repository.
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getUseIncludes()
    bool get_use_includes() const;

    /// Enables/disables using of "includes"
    /// If enabled, only packages listed in the "includepkgs" will be used from the repository.
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.setUseIncludes(bool enabled)
    void set_use_includes(bool enabled);

    /// Returns whether the loading of "other" metadata file is enabled
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getLoadMetadataOther()
    bool get_load_metadata_other() const;

    /// Enables/disables loading of "other" metadata file
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.setLoadMetadataOther(bool value)
    void set_load_metadata_other(bool value);

    /// Returns repository cost
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getCost()
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
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getPriority()
    /// TODO(jrohel): Remove it? It is only shortcut for get_config()->cost()->get_value()
    int get_priority() const;

    /// @brief Set repo priority in RepoConf and in Libsolv repo if attached. Values are only updated when
    /// Option::Priority of stored value is equal or lower.
    ///
    /// @version 1.0.0
    /// @param value Priority value
    /// @param priority Optional argument
    void set_priority(int value, Option::Priority priority = Option::Priority::RUNTIME);

    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getRevision()
    const std::string & get_revision() const;

    /// Gets age of the local cache
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getAge()
    int64_t get_age() const;

    /// Ask for additional repository metadata type to download.
    /// Given metadata are appended to the default metadata set when repository is downloaded
    /// @param metadataType metadata type (filelists, other, productid...)
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.addMetadataTypeToDownload(const std::string &metadataType)
    void add_metadata_type_to_download(const std::string & metadata_type);

    /// Stop asking for this additional repository metadata type.
    /// Given metadata_type is no longer downloaded by default when this repository is downloaded.
    /// TODO(jrohel): Enable better control of downloaded metadata. Add posibility to remove all not only additional metadata.
    /// TODO(jrohel): Add method for get registered metadata type for download.
    /// @param metadataType metadata type (filelists, other, productid...)
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.removeMetadataTypeFromDownload(const std::string &metadataType)
    void remove_metadata_type_from_download(const std::string & metadata_type);

    /// Return path to the particular downloaded repository metadata in cache
    /// @param metadataType metadata type (filelists, other, productid...)
    /// @return file path or empty string in case the requested metadata does not exist
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getMetadataPath(const std::string & metadataType)
    std::string get_metadata_path(const std::string & metadata_type);

    /// Mark whatever is in the current cache expired.
    /// This repo instance will alway try to fetch a fresh metadata after this
    /// method is called.
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.expire()
    void expire();

    /// @brief Return whether the cached metadata is expired.
    /// @return bool
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.isExpired()
    bool is_expired() const;

    /// @brief Get the number of seconds after which the cached metadata will expire.
    /// Negative number means the metadata has expired already.
    /// @return Seconds to expiration
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getExpiresIn()
    int get_expires_in() const;

    /// Gets repository freshness
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.fresh()
    bool fresh();

    /// @replaces libdnf:repo/Repo.hpp:method:Repo.setMaxMirrorTries(int maxMirrorTries)
    void set_max_mirror_tries(int max_mirror_tries);

    /// Gets timestamp of metadata "primary" file
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getTimestamp()
    int64_t get_timestamp() const;

    /// Gets the highest timestamp from repomd records
    /// TODO(jrohel): Used in DNF repolist: displayed as "Repo-updated time" base.py: "using metadata from" in debug messages Is it correct?
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getMaxTimestamp()
    int get_max_timestamp();

    /// Try to preserve remote side timestamps
    /// When set to true the underlying librepo is asked to make an attempt to set the timestamps
    /// of the local downloaded files (repository metadata and packages) to match those from
    /// the remote files.
    /// This feature is by default switched off.
    /// @param preserveRemoteTime true - use remote file timestamp, false - use the current time
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.setPreserveRemoteTime(bool preserveRemoteTime)
    void set_preserve_remote_time(bool preserve_remote_time);

    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getPreserveRemoteTime()
    bool get_preserve_remote_time() const;

    /// TODO(jrohel): Used by DNF repolist. Do we need it?
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getContentTags()
    const std::vector<std::string> & get_content_tags();

    /// TODO(jrohel): Used by DNF repolist. Do we need it?
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getDistroTags()
    const std::vector<std::pair<std::string, std::string>> & get_distro_tags();

    /// Get list of relative locations of metadata files inside the repo
    /// e.g. [('primary', 'repodata/primary.xml.gz'), ('filelists', 'repodata/filelists.xml.gz')...]
    /// @return vector of (metadata_type, location) string pairs
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getMetadataLocations()
    const std::vector<std::pair<std::string, std::string>> get_metadata_locations() const;

    /// Gets path to repository cache directory
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getCacheDir()
    std::string get_cachedir() const;

    /// Gets name of repository
    /// Alias
    std::string get_name() { return this->get_config().name().get_value(); };
    /// Sets repository configuration file path
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.setRepoFilePath(const std::string & path)
    void set_repo_file_path(const std::string & path);

    /// Gets repository configuration file path
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getRepoFilePath()
    const std::string & get_repo_file_path() const noexcept;

    /// Sets repository synchronisation strategy
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.setSyncStrategy(SyncStrategy strategy)
    void set_sync_strategy(SyncStrategy strategy);

    /// Returns repository synchronisation strategy
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getSyncStrategy()
    SyncStrategy get_sync_strategy() const noexcept;

    /// Downloads file from URL into given opened file descriptor.
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.downloadUrl(const char * url, int fd)
    void download_url(const char * url, int fd);

    /// Set http headers.
    /// Example:
    /// {"User-Agent: Agent007", "MyMagicHeader: I'm here", nullptr}
    /// @param headers nullptr terminated array of C strings
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.setHttpHeaders(const char * headers[])
    void set_http_headers(const char * headers[]);

    /// Get array of added/changed/removed http headers.
    /// @return nullptr terminated array of C strings
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getHttpHeaders()
    const char * const * get_http_headers() const;

    /// Returns mirrorlist associated with the repository.
    /// Mirrors on this list are mirrors parsed from mirrorlist/metalink specified by LRO_MIRRORLIST or
    /// from mirrorlist specified by LRO_MIRROSLISTURL and metalink specified by LRO_METALINKURL.
    /// No URLs specified by LRO_URLS are included in this list.
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.getMirrors()
    std::vector<std::string> get_mirrors() const;

    /// Sets substitutions. Substitutions are used to substitute variables in repository configuration.
    /// @replaces libdnf:repo/Repo.hpp:method:Repo.setSubstitutions(const std::map<std::string, std::string> & substitutions)
    void set_substitutions(const std::map<std::string, std::string> & substitutions);

    ~Repo();

private:
    class Impl;
    friend class RepoSack;
    friend class SolvSack;
    friend struct PackageTarget;
    std::unique_ptr<Impl> p_impl;
};

struct Downloader {
public:
    static void download_url(ConfigMain * cfg, const char * url, int fd);
};

/// Base class for PackageTarget callbacks
/// User implements PackageTarget callbacks by inheriting this class and overriding its methods.
class PackageTargetCB {
public:
    // Transfer status codes
    enum class TransferStatus { SUCCESSFUL, ALREADYEXISTS, ERROR };

    virtual int end(TransferStatus status, const char * msg);
    virtual int progress(double total_to_download, double downloaded);
    virtual int mirror_failure(const char * msg, const char * url);
    virtual ~PackageTargetCB() = default;
};

/// Wraps librepo PackageTarget
struct PackageTarget {
public:
    // Enum of supported checksum types.
    // NOTE! This enum guarantee to be sorted by "hash quality"
    // TODO(jrohel): Use the same enum/checksum utilities on all places.
    enum class ChecksumType {
        UNKNOWN,
        MD5,     //    The most weakest hash
        SHA1,    //  |
        SHA224,  //  |
        SHA256,  //  |
        SHA384,  // \|/
        SHA512,  //    The most secure hash
    };

    /// TODO(jrohel): Use the same checksum utilities on all places.
    /// @replaces libdnf:repo/Repo.hpp:method:PacketTarget.checksumType(const std::string & name)
    static ChecksumType checksum_type(const std::string & name);

    /// @replaces libdnf:repo/Repo.hpp:method:PacketTarget.downloadPackages(std::vector<PackageTarget *> & targets, bool failFast)
    static void download_packages(std::vector<PackageTarget *> & targets, bool fail_fast);

    PackageTarget(
        Repo * repo,
        const char * relative_url,
        const char * dest,
        int chks_type,
        const char * chksum,
        int64_t expected_size,
        const char * base_url,
        bool resume,
        int64_t byte_range_start,
        int64_t byte_range_end,
        PackageTargetCB * callbacks);
    PackageTarget(
        ConfigMain * cfg,
        const char * relative_url,
        const char * dest,
        int chks_type,
        const char * chksum,
        int64_t expected_size,
        const char * base_url,
        bool resume,
        int64_t byte_range_start,
        int64_t byte_range_end,
        PackageTargetCB * callbacks,
        const char * http_headers[] = nullptr);
    ~PackageTarget();

    /// Returns a pointer to the registered class that implements callback methods (download state).
    /// @replaces libdnf:repo/Repo.hpp:method:PackageTarget.getCallbacks()
    PackageTargetCB * get_callbacks();

    /// Returns last error message
    /// TODO(jrohel): Do we want the method?
    /// @replaces libdnf:repo/Repo.hpp:method:PackageTarget.getErr()
    const char * get_err();

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;
};

/// Logger for librepo
/// TODO(jrohel): Rewrite to use global logger (LogRouter)
struct LibrepoLog {
public:
    /// @replaces libdnf:repo/Repo.hpp:method:LibrepoLog.addHandler(const std::string & filePath, bool debug)
    static long add_handler(const std::string & file_path, bool debug = false);

    /// @replaces libdnf:repo/Repo.hpp:method:LibrepoLog.removeAllHandlers()
    static void remove_handler(long uid);

    /// @replaces libdnf:repo/Repo.hpp:method:LibrepoLog.removeHandler(long uid)
    static void remove_all_handlers();
};

}  // namespace libdnf::rpm

#endif
