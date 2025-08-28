// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_REPOQUERY_HPP
#define LIBDNF5_CLI_OUTPUT_REPOQUERY_HPP

#include "libdnf5-cli/defs.h"

#include <libdnf5/rpm/package_set.hpp>

namespace libdnf5::cli::output {

LIBDNF_CLI_API bool requires_filelists(const std::string & queryformat);

LIBDNF_CLI_API void print_pkg_set_with_format(
    std::FILE * target, const libdnf5::rpm::PackageSet & pkgs, const std::string & queryformat);

LIBDNF_CLI_API void print_pkg_attr_uniq_sorted(
    std::FILE * target, const libdnf5::rpm::PackageSet & pkgs, const std::string & getter_name);

LIBDNF_CLI_API void print_available_pkg_attrs(std::FILE * target);

LIBDNF_CLI_API libdnf5::rpm::ReldepList get_reldeplist_for_attr(
    const libdnf5::rpm::PackageSet & pkgs, const std::string & getter_name);

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_REPOQUERY_HPP
