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


#ifndef LIBDNF5_CLI_OUTPUT_ADAPTERS_REPO_TMPL_HPP
#define LIBDNF5_CLI_OUTPUT_ADAPTERS_REPO_TMPL_HPP

#include "../interfaces/repo.hpp"

namespace libdnf5::cli::output {

template <class T>
class RepoAdapter : public IRepo {
public:
    RepoAdapter(const T & repo) : repo{repo} {}

    RepoAdapter(T && repo) : repo{std::move(repo)} {}

    std::string get_id() const override { return repo->get_id(); }

    bool is_enabled() const override { return repo->is_enabled(); }

    std::string get_name() const override { return repo->get_config().get_name_option().get_value(); }

private:
    T repo;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_ADAPTERS_REPO_TMPL_HPP
