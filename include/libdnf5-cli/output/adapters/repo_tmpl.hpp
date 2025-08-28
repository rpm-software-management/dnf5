// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


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
