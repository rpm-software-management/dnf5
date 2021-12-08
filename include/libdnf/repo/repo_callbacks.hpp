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


namespace libdnf::repo {

/// Base class for repository metadata download callbacks.
/// To implement callbacks, inherit from this class and override virtual methods.
class RepoCallbacks {
public:
    enum class FastestMirrorStage {
        /// Fastest mirror detection just started. ptr is NULL.
        INIT,

        /// ptr is a (char *) pointer to a string with path to the cache file.
        /// Do not modify or free the string.
        CACHELOADING,

        /// If cache was loaded successfully, ptr is NULL, otherwise ptr is a
        /// (char *) string with error message. Do not modify or free the string.
        CACHELOADINGSTATUS,

        /// Detection (pinging) in progress. If all data was loaded from cache,
        /// this stage is skiped. ptr is a pointer to long, the number mirrors
        /// which will be tested.
        DETECTION,

        /// Detection is done, sorting mirrors, updating cache, etc. ptr is NULL.
        FINISHING,

        /// The very last invocation of the fastest mirror callback. If fastest
        /// mirror detection was successful, ptr is NULL. Otherwise ptr points
        /// to a string with an error message. Do not modify or free the string.
        STATUS,
    };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    virtual void start(const char * what) {}
    virtual void end() {}
    virtual int progress(double total_to_download, double downloaded) { return 0; }
    virtual void fastest_mirror(FastestMirrorStage stage, const char * ptr) {}
    virtual int handle_mirror_failure(const char * msg, const char * url, const char * metadata) { return 0; }
    virtual bool repokey_import(
        const std::string & id,
        const std::string & user_id,
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
