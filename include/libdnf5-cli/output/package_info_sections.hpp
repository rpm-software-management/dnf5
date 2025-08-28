// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_PACKAGE_INFO_SECTIONS_HPP
#define LIBDNF5_CLI_OUTPUT_PACKAGE_INFO_SECTIONS_HPP

#include "package_list_sections.hpp"

#include "libdnf5-cli/defs.h"

namespace libdnf5::cli::output {

class LIBDNF_CLI_API PackageInfoSections : public PackageListSections {
public:
    PackageInfoSections();
    ~PackageInfoSections();

    /// Print the table
    /// @param colorizer Optional class to select color for packages in output
    void print(const std::unique_ptr<PkgColorizer> & colorizer = nullptr) override;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_PACKAGE_INFO_SECTIONS_HPP
