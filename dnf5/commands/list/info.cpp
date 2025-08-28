// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "info.hpp"

#include <libdnf5-cli/output/package_info_sections.hpp>

namespace dnf5 {

using namespace libdnf5::cli;

std::unique_ptr<libdnf5::cli::output::PackageListSections> InfoCommand::create_output() {
    auto out = std::make_unique<libdnf5::cli::output::PackageInfoSections>();
    return out;
}

}  // namespace dnf5
