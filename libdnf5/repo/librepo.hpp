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

#ifndef LIBDNF5_REPO_LIBREPO_PRIVATE_HPP
#define LIBDNF5_REPO_LIBREPO_PRIVATE_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/repo/config_repo.hpp"

#include <librepo/librepo.h>

#include <memory>

namespace std {

template <>
struct default_delete<GError> {
    void operator()(GError * ptr) noexcept { g_error_free(ptr); }
};

}  // namespace std


namespace libdnf5::repo {

class LibrepoError : public Error {
public:
    LibrepoError(std::unique_ptr<GError> && lr_error);
    const char * get_domain_name() const noexcept override { return "libdnf5::repo"; }
    const char * get_name() const noexcept override { return "LibrepoError"; }
    int get_code() const noexcept { return code; }

private:
    int code;
};


class LibrepoResult {
public:
    LibrepoResult() : result(lr_result_init()) {}
    LibrepoResult(const LibrepoResult & other) = delete;
    LibrepoResult(LibrepoResult && other) noexcept : result(other.result) { other.result = nullptr; }
    ~LibrepoResult() { lr_result_free(result); }

    LibrepoResult & operator=(const LibrepoResult & other) = delete;
    LibrepoResult & operator=(LibrepoResult && other) noexcept;

    template <typename T>
    void get_info(LrResultInfoOption option, T * value) {
        GError * err_p{nullptr};
        if (!lr_result_getinfo(result, &err_p, option, value)) {
            throw libdnf5::repo::LibrepoError(std::unique_ptr<GError>(err_p));
        }
    }

private:
    friend class LibrepoHandle;
    LrResult * get() { return result; }

    LrResult * result;
};


class LibrepoHandle {
public:
    LibrepoHandle() : handle(lr_handle_init()) {}
    LibrepoHandle(const LibrepoHandle & other) = delete;
    LibrepoHandle(LibrepoHandle && other) noexcept : handle(other.handle) { other.handle = nullptr; }
    ~LibrepoHandle() { lr_handle_free(handle); }

    LibrepoHandle & operator=(const LibrepoHandle & other) = delete;
    LibrepoHandle & operator=(LibrepoHandle && other) noexcept;

    void init_remote(const libdnf5::ConfigMain & config);
    void init_remote(const libdnf5::repo::ConfigRepo & config);

    template <typename T>
    void set_opt(LrHandleOption option, T value) {
        GError * err_p{nullptr};
        if (!lr_handle_setopt(handle, &err_p, option, value)) {
            throw libdnf5::repo::LibrepoError(std::unique_ptr<GError>(err_p));
        }
    }

    template <typename T>
    void get_info(LrHandleInfoOption option, T * value) {
        GError * err_p{nullptr};
        if (!lr_handle_getinfo(handle, &err_p, option, value)) {
            throw libdnf5::repo::LibrepoError(std::unique_ptr<GError>(err_p));
        }
    }

    LibrepoResult perform();

    LrHandle * get() { return handle; }

private:
    LrHandle * handle;
};

}  // namespace libdnf5::repo

#endif  // LIBDNF5_REPO_LIBREPO_PRIVATE_HPP
