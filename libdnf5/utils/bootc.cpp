// Copyright Contributors to the DNF5 project.
// Copyright (C) 2025 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "libdnf5/utils/bootc.hpp"

#include "utils/subprocess.hpp"

#include "libdnf5/common/exception.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <json.h>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <sched.h>
#include <sys/statvfs.h>

#include <cerrno>
#include <memory>
#include <string>

namespace libdnf5::utils::bootc {

std::unique_ptr<json_object, decltype(&json_object_put)> get_bootc_status() {
    const auto & result = subprocess::run("bootc", {"bootc", "status", "--format=json"});

    // Check whether command failed (non-zero exit code)
    if (result.returncode != 0) {
        // Command ran but failed; show stderr to user
        std::string stderr_content(reinterpret_cast<const char *>(result.stderr.data()), result.stderr.size());
        if (!stderr_content.empty()) {
            throw libdnf5::RuntimeError(M_("Error checking bootc status: {}"), stderr_content);
        } else {
            throw libdnf5::RuntimeError(
                M_("Error checking bootc status: bootc command failed with exit code {}"), result.returncode);
        }
    }

    std::string stdout_content(reinterpret_cast<const char *>(result.stdout.data()), result.stdout.size());
    std::unique_ptr<json_object, decltype(&json_object_put)> root(
        json_tokener_parse(stdout_content.c_str()), json_object_put);

    if (!root) {
        throw libdnf5::RuntimeError(M_("Error checking bootc status: Failed to parse JSON output"));
    }

    return root;
}

bool is_bootc_system() {
    std::unique_ptr<json_object, decltype(&json_object_put)> bootc_status{nullptr, json_object_put};

    try {
        bootc_status = get_bootc_status();
    } catch (const libdnf5::SystemError & ex) {
        if (ex.get_error_code() == ENOENT) {
            return false;
        }
        throw libdnf5::RuntimeError(M_("Failed to query status of bootc system: {}"), std::string(ex.what()));
    }

    json_object * spec_obj = nullptr;
    if (!json_object_object_get_ex(bootc_status.get(), "spec", &spec_obj)) {
        return false;
    }
    json_object * image_obj = nullptr;
    if (!json_object_object_get_ex(spec_obj, "image", &image_obj)) {
        return false;
    }
    return !json_object_is_type(image_obj, json_type_null);
}

bool is_writable() {
    struct statvfs vfs;
    if (statvfs("/usr", &vfs) != 0) {
        throw libdnf5::SystemError(errno, M_("Failed to stat /usr filesystem"));
    }
    return (vfs.f_flag & ST_RDONLY) == 0;
}

bool has_read_only_usr_overlay() {
    const auto & bootc_status = get_bootc_status();

    json_object * status_obj = nullptr;
    if (!json_object_object_get_ex(bootc_status.get(), "status", &status_obj)) {
        return false;
    }
    json_object * usr_overlay_obj = nullptr;
    if (!json_object_object_get_ex(status_obj, "usrOverlay", &usr_overlay_obj)) {
        return false;
    }
    json_object * access_mode_obj = nullptr;
    if (!json_object_object_get_ex(usr_overlay_obj, "accessMode", &access_mode_obj)) {
        return false;
    }
    const char * access_mode = json_object_get_string(access_mode_obj);
    return access_mode && std::string{access_mode} == "readOnly";
}

void make_usr_writable() {
    if (is_writable()) {
        return;
    }

    if (!has_read_only_usr_overlay()) {
        // Set up transient overlay
        const auto & result = subprocess::run("bootc", {"bootc", "usr-overlay", "--read-only"});
        if (result.returncode != 0) {
            std::string stderr_content(reinterpret_cast<const char *>(result.stderr.data()), result.stderr.size());
            if (!stderr_content.empty()) {
                throw libdnf5::RuntimeError(M_("Error running `bootc usr-overlay --read-only`: {}"), stderr_content);
            } else {
                throw libdnf5::RuntimeError(
                    M_("Error running `bootc usr-overlay --read-only`: bootc command failed with exit code {}"),
                    result.returncode);
            }
        }
    }

    // Set up mountns
    if (unshare(CLONE_NEWNS) != 0) {
        throw libdnf5::SystemError(errno, M_("Failed to unshare mount namespace"));
    }

    const auto & result = subprocess::run("mount", {"mount", "--options-source=disable", "-o", "remount,rw", "/usr"});

    if (result.returncode != 0) {
        std::string stderr_content(reinterpret_cast<const char *>(result.stderr.data()), result.stderr.size());
        throw libdnf5::RuntimeError(M_("Failed to remount /usr as read-write: {}"), stderr_content);
    }
}

}  // namespace libdnf5::utils::bootc
