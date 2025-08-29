// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_REPO_INFO_HPP
#define LIBDNF5_CLI_OUTPUT_REPO_INFO_HPP

#include "interfaces/repo.hpp"

#include "libdnf5-cli/defs.h"

#include <memory>

namespace libdnf5::cli::output {

class LIBDNF_CLI_API RepoInfo {
public:
    RepoInfo();
    ~RepoInfo();

    void add_repo(IRepoInfo & repo);
    void print();

private:
    class LIBDNF_CLI_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

LIBDNF_CLI_API void print_repoinfo_json(const std::vector<std::unique_ptr<IRepoInfo>> & repos);

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_REPO_INFO_HPP
