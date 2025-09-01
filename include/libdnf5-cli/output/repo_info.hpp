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
