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

#ifndef LIBDNF_REPO_DOWNLOAD_CALLBACKS_HPP
#define LIBDNF_REPO_DOWNLOAD_CALLBACKS_HPP

#include <libdnf/rpm/package.hpp>

#include <memory>
#include <string>

namespace libdnf::repo {

/// Base class for download callbacks.
/// To implement a callback, inherit from this class and override the virtual methods.
class DownloadCallbacks {
public:
    /// Transfer status codes.
    enum class TransferStatus { SUCCESSFUL, ALREADYEXISTS, ERROR };

    virtual ~DownloadCallbacks() = default;

    /// End of download callback.
    /// @param status The transfer status.
    /// @param msg The error message in case of error.
    /// @return TODO(lukash) uses the LrCbReturnCode enum from librepo, we should translate that.
    virtual int end(TransferStatus status, const char * msg);

    /// Download progress callback.
    /// @param total_to_download Total number of bytes to download.
    /// @param downloaded Number of bytes downloaded.
    /// @return TODO(lukash) uses the LrCbReturnCode enum from librepo, we should translate that.
    virtual int progress(double total_to_download, double downloaded);

    /// Mirror failure callback.
    /// @param msg Error message.
    /// @param url Failed mirror URL.
    /// @return TODO(lukash) uses the LrCbReturnCode enum from librepo, we should translate that.
    virtual int mirror_failure(const char * msg, const char * url);
};


/// Interface of DownloadCallbacks factory class.
/// User of API is supposed to create a subclass of this class and register the
/// instance in the Base.  This will be then used to create instances of concrete
/// DownloadCallbacks subclass.
/// For download callbacks we have two, kind of contradicting, requirements:
/// 1. callbacks need to be instantiated by libdnf5 library, they cannot be instantiated
/// by the user and passed to the library. The reason is duplicating the download
/// code in several parts of client applications.
/// 2. callbacks need access to some elements created in the client application -
/// for example instance of libdnf::cli::progressbar::MultiProgressBar.
class DownloadCallbacksFactory {
public:
    virtual ~DownloadCallbacksFactory(){};

    // Create a DownloadCallbacks (or it's subclass) instance based on the string description.
    // @param what  String which describes what is being downloaded (URL, package name...)
    virtual std::unique_ptr<DownloadCallbacks> create_callbacks(const std::string & what);

    // Create a DownloadCallbacks (or it's subclass) instance based on the rpm package object.
    // @param package  rpm::Package object that is being downloaded
    virtual std::unique_ptr<DownloadCallbacks> create_callbacks(const libdnf::rpm::Package & package);
};


}  // namespace libdnf::repo

#endif  // LIBDNF_REPO_DOWNLOAD_CALLBACKS_HPP
