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

#ifndef DNF5_COMMANDS_COPR_CONSTANTS_HPP
#define DNF5_COMMANDS_COPR_CONSTANTS_HPP

#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

static const char * const COPR_COMMAND_DESCRIPTION =
    _("Manage Copr repositories (add-ons provided by users/community/third-party)");
static const char * const COPR_THIRD_PARTY_WARNING =
    _("Enabling a Copr repository. Please note that this repository is not part\n"
      "of the main distribution, and quality may vary.\n"
      "\n"
      "The Fedora Project does not exercise any power over the contents of\n"
      "this repository beyond the rules outlined in the Copr FAQ at\n"
      "<https://docs.pagure.org/copr.copr/user_documentation.html#what-i-can-build-in-copr>,\n"
      "and packages are not held to any quality or security level.\n"
      "\n"
      "Please do not file bug reports about these packages in Fedora\n"
      "Bugzilla. In case of problems, contact the owner of this repository.\n");

static const char * const COPR_EXTERNAL_DEPS_WARNING =
    _("Maintainer of the enabled Copr repository decided to make\n"
      "it dependent on other repositories. Such repositories are\n"
      "usually necessary for successful installation of RPMs from\n"
      "the main Copr repository (they provide runtime dependencies).\n"
      "\n"
      "Be aware that the note about quality and bug-reporting\n"
      "above applies here too, Fedora Project doesn't control the\n"
      "content. Please review the list:\n"
      "\n"
      "{}"
      "\n"
      "These repositories are being enabled together with the main\n"
      "repository.\n");


constexpr const char * COPR_DEFAULT_HUB = "copr.fedorainfracloud.org";
constexpr const char * COPR_DNF5_COMMAND = "dnf5";
constexpr const char * COPR_REPO_DIRECTORY = "/etc/yum.repos.d";


#endif  // DNF5_COMMANDS_COPR_CONSTANTS_HPP
