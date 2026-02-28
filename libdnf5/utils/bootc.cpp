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
#include <sys/statvfs.h>

#include <cerrno>
#include <iostream>
#include <memory>
#include <string>

namespace libdnf5::utils::bootc {

bool is_bootc_system() {
    // Per https://raw.githubusercontent.com/bootc-dev/bootc/11cba840a4cc6dca9afc528f306ebe1406b647fa/docs/src/package-managers.md,
    // check `bootc status --format=json` has non-null .spec.image to determine whether bootc is in use.

    CompletedProcess result;
    try {
        result = run("bootc", {"bootc", "status", "--format=json"});
    } catch (const libdnf5::SystemError & ex) {
        if (ex.get_error_code() == ENOENT) {
            return false;
        }
        throw libdnf5::RuntimeError(M_("Failed to query status of bootc system: {}"), std::string(ex.what()));
    }

    // Check if command failed (non-zero exit code)
    if (result.returncode != 0) {
        // Command ran but failed - show stderr to user
        std::string stderr_content(reinterpret_cast<const char *>(result.stderr.data()), result.stderr.size());
        if (!stderr_content.empty()) {
            throw libdnf5::RuntimeError(M_("Got the following error checking bootc status: {}"), stderr_content);
        } else {
            throw libdnf5::RuntimeError(
                M_("Got the following error checking bootc status: bootc command failed with exit code {}"),
                result.returncode);
        }
    }

    // Parse JSON output to check if .spec.image is not null
    std::string stdout_content(reinterpret_cast<const char *>(result.stdout.data()), result.stdout.size());
    std::unique_ptr<json_object, decltype(&json_object_put)> root(
        json_tokener_parse(stdout_content.c_str()), json_object_put);

    if (!root) {
        // Failed to parse JSON
        throw libdnf5::RuntimeError(M_("Got the following error checking bootc status: Failed to parse JSON output"));
    }

    json_object * spec_obj = nullptr;
    if (!json_object_object_get_ex(root.get(), "spec", &spec_obj)) {
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

}  // namespace libdnf5::utils::bootc
