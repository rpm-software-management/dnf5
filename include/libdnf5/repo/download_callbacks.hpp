// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBDNF5_REPO_DOWNLOAD_CALLBACKS_HPP
#define LIBDNF5_REPO_DOWNLOAD_CALLBACKS_HPP

#include "libdnf5/defs.h"

namespace libdnf5 {

class Base;

}

namespace libdnf5::repo {

/// Base class for download callbacks.
/// To implement a callback, inherit from this class and override the virtual methods.
class LIBDNF_API DownloadCallbacks {
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
        /// this stage is skipped. `ptr` is a pointer to `long`, the number of
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

    /// Transfer status codes.
    enum class TransferStatus { SUCCESSFUL, ALREADYEXISTS, ERROR };

    /// Download callbacks return values.
    /// RETURN_OK - all is fine, RETURN_ABORT - abort just the current download, RETURN_ERROR - abort all downloading
    enum ReturnCode : int { OK = 0, ABORT = 1, ERROR = 2 };

    DownloadCallbacks() = default;
    DownloadCallbacks(const DownloadCallbacks &) = delete;
    DownloadCallbacks(DownloadCallbacks &&) = delete;
    DownloadCallbacks & operator=(const DownloadCallbacks &) = delete;
    DownloadCallbacks & operator=(DownloadCallbacks &&) = delete;
    virtual ~DownloadCallbacks() = default;

    /// Notify the client that a new download has been created.
    /// @param user_data User data entered together with url/package to download.
    /// @param description The message describing new download (url/packagename).
    /// @param total_to_download Total number of bytes to download.
    /// @return Associated user data for new download.
    virtual void * add_new_download(void * user_data, const char * description, double total_to_download);

    /// Download progress callback.
    /// @param user_cb_data Associated user data obtained from add_new_download.
    /// @param total_to_download Total number of bytes to download.
    /// @param downloaded Number of bytes downloaded.
    /// @return TODO(lukash) uses the LrCbReturnCode enum from librepo, we should translate that.
    virtual int progress(void * user_cb_data, double total_to_download, double downloaded);

    /// End of download callback.
    /// @param user_cb_data Associated user data obtained from add_new_download.
    /// @param status The transfer status.
    /// @param msg The error message in case of error.
    /// @return TODO(lukash) uses the LrCbReturnCode enum from librepo, we should translate that.
    virtual int end(void * user_cb_data, TransferStatus status, const char * msg);

    /// Mirror failure callback.
    /// @param user_cb_data Associated user data obtained from add_new_download.
    /// @param msg Error message.
    /// @param url Failed mirror URL.
    /// @param metadata the type of metadata that is being downloaded TODO(lukash) should this point to LoadFlags in some way?
    /// @return TODO(lukash) uses the LrCbReturnCode enum from librepo, we should translate that.
    virtual int mirror_failure(void * user_cb_data, const char * msg, const char * url, const char * metadata);

    // TODO(lukash) needs more specifics about how the detection works
    /// Callback for fastest mirror detection.
    /// @param user_cb_data Associated user data obtained from new_download.
    /// @param stage the stage of the fastest mirror detection, refer to `FastestMirrorStage`
    /// @param ptr pointer to additional data depending on the stage, refer to `FastestMirrorStage`
    // TODO(lukash) the varying data type for ptr will not work with bindings, we need unambiguous API
    virtual void fastest_mirror(void * user_cb_data, FastestMirrorStage stage, const char * ptr);
};

}  // namespace libdnf5::repo

#endif  // LIBDNF5_REPO_DOWNLOAD_CALLBACKS_HPP
