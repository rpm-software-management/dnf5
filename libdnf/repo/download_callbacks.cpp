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

#include "libdnf/repo/download_callbacks.hpp"

namespace libdnf::repo {

int DownloadCallbacks::end([[maybe_unused]] TransferStatus status, [[maybe_unused]] const char * msg) {
    return 0;
}
int DownloadCallbacks::progress([[maybe_unused]] double total_to_download, [[maybe_unused]] double downloaded) {
    return 0;
}
int DownloadCallbacks::mirror_failure([[maybe_unused]] const char * msg, [[maybe_unused]] const char * url) {
    return 0;
}

std::unique_ptr<DownloadCallbacks> DownloadCallbacksFactory::create_callbacks(
    [[maybe_unused]] const std::string & what) {
    return nullptr;
}

std::unique_ptr<DownloadCallbacks> DownloadCallbacksFactory::create_callbacks(const libdnf::rpm::Package & package) {
    return create_callbacks(package.get_full_nevra());
}

}  // namespace libdnf::repo
