// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "versionlock_clear.hpp"

#include <libdnf5/utils/bgettext/bgettext-lib.h>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void VersionlockClearCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Remove all entries from versionlock configuration"));
}

void VersionlockClearCommand::run() {
    auto & ctx = get_context();
    auto package_sack = ctx.get_base().get_rpm_package_sack();
    auto vl_config = package_sack->get_versionlock_config();
    vl_config.get_packages().clear();
    vl_config.save();
}

}  // namespace dnf5
