// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef DNF5_COMMANDS_OBS_REPO_HPP
#define DNF5_COMMANDS_OBS_REPO_HPP

#include "html.hpp"
#include "obs_config.hpp"
#include "obs_constants.hpp"

#include <libdnf5/base/base.hpp>

#include <filesystem>
#include <regex>


namespace dnf5 {

class ObsRepo {
public:
    explicit ObsRepo(
        libdnf5::Base & base,
        const std::string & project_repo_spec);

    explicit ObsRepo(
        libdnf5::Base & base,
        const std::filesystem::path & repo_file_path);

    void save();
    void remove() const;
    void disable();
    void enable();
    void save_interactive();

    bool is_enabled() const;
    std::string get_id() const;

    std::filesystem::path get_repo_file_path() const { return repo_file_path; };

private:
    libdnf5::Base * base{nullptr};
    libdnf5::ConfigParser parser;

    std::filesystem::path repo_file_path;

    std::string hostname = "";
    std::string project = "";
    std::string reponame = "";
};

using ObsRepoCallback = std::function<void(ObsRepo &)>;
void installed_obs_repositories(libdnf5::Base & base, ObsRepoCallback cb);
std::filesystem::path obs_repo_directory(libdnf5::Base * base);

void obs_repo_disable(libdnf5::Base & base, const std::string & repo_spec);
void obs_repo_remove(libdnf5::Base & base, const std::string & repo_spec);

}  // namespace dnf5

#endif  // DNF5_COMMANDS_OBS_REPO_HPP
