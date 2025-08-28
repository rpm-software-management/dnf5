// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_PACKAGE_LIST_SECTIONS_IMPL_HPP
#define LIBDNF5_CLI_OUTPUT_PACKAGE_LIST_SECTIONS_IMPL_HPP

#include "libdnf5-cli/output/package_list_sections.hpp"


namespace libdnf5::cli::output {

class PackageListSections::Impl {
public:
    std::vector<std::tuple<
        std::string,
        libdnf5::rpm::PackageSet,
        std::map<libdnf5::rpm::PackageId, std::vector<libdnf5::rpm::Package>>>>
        sections;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_PACKAGE_LIST_SECTIONS_IMPL_HPP
