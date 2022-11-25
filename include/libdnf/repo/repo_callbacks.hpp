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

#ifndef LIBDNF_REPO_REPO_CALLBACKS_HPP
#define LIBDNF_REPO_REPO_CALLBACKS_HPP

#include <string>
#include <vector>

namespace libdnf::repo {

/// Base class for repository metadata download callbacks.
/// To implement callbacks, inherit from this class and override virtual methods.
class RepoCallbacks {
public:
    enum class FastestMirrorStage {
        /// Fastest mirror detection started. ptr is `nullptr`.
        INIT,

        // TODO(lukash) what does CACHELOADING mean?
        /// `ptr` is a (`char *`) pointer to a string with path to the cache file.
        /// Do not modify or free the string.
        CACHELOADING,

        /// If cache was loaded successfully, `ptr` is `nullptr`, otherwise it
        /// is a (`char *`) string containing the error message. Do not modify
        /// or free the string.
        CACHELOADINGSTATUS,

        /// Detection (pinging) in progress. If all data was loaded from cache,
        /// this stage is skiped. `ptr` is a pointer to `long`, the number of
        /// mirrors which will be tested.
        DETECTION,

        /// Detection is done, sorting mirrors, updating cache, etc. `ptr` is
        /// `nullptr`.
        FINISHING,

        /// The last invocation of the fastest mirror callback. If detection
        /// was successful, `ptr` is `nullptr`. Otherwise it is a (`char *`) /
        /// string containing the error message. Do not modify or free the
        /// string.
        STATUS,
    };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

    /// Start of download callback.
    /// @param what The name (if set) or the id of the repository
    virtual void start(const char * what) {}

    /// End of download callback.
    /// @param error_message `nullptr` on success, otherwise the error message
    virtual void end(const char * error_message) {}

    /// Download progress callback.
    /// @param total_to_download total amount of bytes to download
    /// @param downloaded bytes downloaded
    /// @return TODO(lukash) this uses librepo's LrCbReturnCode, we should have our own enum
    virtual int progress(double total_to_download, double downloaded) { return 0; }

    // TODO(lukash) needs more specifics about how the detection works
    /// Callback for fastest mirror detection.
    /// @param stage the stage of the fastest mirror detection, refer to `FastestMirrorStage`
    /// @param ptr pointer to additional data depending on the stage, refer to `FastestMirrorStage`
    // TODO(lukash) the varying data type for ptr will not work with bindings, we need unambiguous API
    virtual void fastest_mirror(FastestMirrorStage stage, const char * ptr) {}

    /// Mirror failure callback. Called when downloading from a mirror failed.
    /// @param msg the error message
    /// @param url the mirror url
    /// @param metadata the type of metadata that is being downloaded TODO(lukash) should this point to LoadFlags in some way?
    /// @return TODO(lukash) this uses librepo's LrCbReturnCode, we should have our own enum
    virtual int handle_mirror_failure(const char * msg, const char * url, const char * metadata) { return 0; }

    /// GPG key import callback. Allows to confirm or deny the import.
    /// @param id the key id
    /// @param user_ids the list of the key user IDs
    /// @param fingerprint the fingerprint of the key
    /// @param url the URL from which the key was downloaded
    /// @param timestamp the timestamp of the key
    /// @return `true` to import the key, `false` to not import
    virtual bool repokey_import(
        const std::string & id,
        const std::vector<std::string> & user_ids,
        const std::string & fingerprint,
        const std::string & url,
        long int timestamp) {
        return true;
    }
#pragma GCC diagnostic pop

    virtual ~RepoCallbacks() = default;
};


}  // namespace libdnf::repo

#endif  // LIBDNF_REPO_REPO_CALLBACKS_HPP
