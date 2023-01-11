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

#ifndef LIBDNF_BASE_DOWNLOAD_CALLBACKS_HPP
#define LIBDNF_BASE_DOWNLOAD_CALLBACKS_HPP

#include <string>

namespace libdnf {

struct UserDownloadCallbackData;

class Base;

/// Base class for download callbacks.
/// To implement a callback, inherit from this class and override the virtual methods.
class DownloadCallbacks {
public:
    /// Transfer status codes.
    enum class TransferStatus { SUCCESSFUL, ALREADYEXISTS, ERROR };

    DownloadCallbacks(Base & base) : base(&base) {}

    virtual ~DownloadCallbacks() = default;

    /// Notify the client that a new download has been created.
    /// @param msg The message describing new download (filename).
    /// @return Associated user data for new download.
    virtual UserDownloadCallbackData * new_download(const std::string & msg);

    /// Download progress callback.
    /// @param user_data Associated user data obtained from new_download.
    /// @param total_to_download Total number of bytes to download.
    /// @param downloaded Number of bytes downloaded.
    /// @return TODO(lukash) uses the LrCbReturnCode enum from librepo, we should translate that.
    virtual int progress(UserDownloadCallbackData * user_data, double total_to_download, double downloaded);

    /// End of download callback.
    /// @param user_data Associated user data obtained from new_download.
    /// @param status The transfer status.
    /// @param msg The error message in case of error.
    /// @return TODO(lukash) uses the LrCbReturnCode enum from librepo, we should translate that.
    virtual int end(UserDownloadCallbackData * user_data, TransferStatus status, const char * msg);


    /// Mirror failure callback.
    /// @param user_data Associated user data obtained from new_download.
    /// @param msg Error message.
    /// @param url Failed mirror URL.
    /// @return TODO(lukash) uses the LrCbReturnCode enum from librepo, we should translate that.
    virtual int mirror_failure(UserDownloadCallbackData * user_data, const char * msg, const char * url);

private:
    Base * base;
};


}  // namespace libdnf

#endif
