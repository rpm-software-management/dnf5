// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_ADVISORYLIST_HPP
#define LIBDNF5_CLI_OUTPUT_ADVISORYLIST_HPP

#include "interfaces/advisory.hpp"

#include "libdnf5-cli/defs.h"

#include <libdnf5/advisory/advisory_package.hpp>
#include <libdnf5/advisory/advisory_reference.hpp>

namespace libdnf5::cli::output {

LIBDNF_CLI_API void print_advisorylist_table(
    std::vector<std::unique_ptr<IAdvisoryPackage>> & advisory_package_list_not_installed,
    std::vector<std::unique_ptr<IAdvisoryPackage>> & advisory_package_list_installed);

LIBDNF_CLI_API void print_advisorylist_json(
    std::vector<std::unique_ptr<IAdvisoryPackage>> & advisory_package_list_not_installed,
    std::vector<std::unique_ptr<IAdvisoryPackage>> & advisory_package_list_installed);

LIBDNF_CLI_API void print_advisorylist_references_table(
    std::vector<libdnf5::advisory::AdvisoryPackage> & advisory_package_list_not_installed,
    std::vector<libdnf5::advisory::AdvisoryPackage> & advisory_package_list_installed,
    std::string reference_type);

LIBDNF_CLI_API void print_advisorylist_references_json(
    std::vector<libdnf5::advisory::AdvisoryPackage> & advisory_package_list_not_installed,
    std::vector<libdnf5::advisory::AdvisoryPackage> & advisory_package_list_installed,
    std::string reference_type);

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_ADVISORYLIST_HPP
