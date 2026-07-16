// Copyright Contributors to the DNF5 project.
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

#ifndef DNF5_COMMANDS_OBS_CONSTANTS_HPP
#define DNF5_COMMANDS_OBS_CONSTANTS_HPP

#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

#include <utility>

static const char * const OBS_COMMAND_DESCRIPTION =
    _("Manage OBS repositories (add-ons provided by users/community/third-party)");
static const char * const OBS_THIRD_PARTY_WARNING =
    _("Enabling an OBS repository. Please note that this repository is not part\n"
      "of the main distribution, and quality may vary.\n"
      "\n"
      "The openSUSE Project does not exercise any power over the contents of\n"
      "this repository, and packages are not held to any quality or security\n"
      "level.\n"
      "\n"
      "Please do not file bug reports about these packages in openSUSE\n"
      "Bugzilla. In case of problems, contact the owner of this repository.\n");


constexpr const char * OBS_REPO_DIRECTORY = "/etc/yum.repos.d";

constexpr const char * OBS_DEFAULT_HUBSPEC = "opensuse";

#define OBS_OPENSUSE_HOSTNAME "build.opensuse.org"
#define OBS_OPENSUSE_DOWNLOAD_HOSTNAME "download.opensuse.org"

constexpr std::pair<const char *, const char *> OBS_OPENSUSE_CONFIG_OPTIONS[] = {
    {"protocol", "https"},
    {"port", "443"},
    {"url_prefix", "/project/repository_state/"},
    {"download_hostname", OBS_OPENSUSE_DOWNLOAD_HOSTNAME},
    {"download_protocol", "https"},
    {"download_port", "443"},
    {"download_url_prefix", "/repositories/"}
};

constexpr const char * OBS_DEFAULT_SECTION = "DEFAULT";

constexpr std::pair<const char *, const char *> OBS_DEFAULT_CONFIG_OPTIONS[] = {
    {"protocol", "https"},
    {"url_prefix", "/project/repository_state/"},
    {"download_protocol", "http"},
    {"download_port", "82"}
};


#endif  // DNF5_COMMANDS_OBS_CONSTANTS_HPP
