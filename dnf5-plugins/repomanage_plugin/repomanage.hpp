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


#ifndef DNF5_PLUGINS_REPOMANAGE_PLUGIN_REPOMANAGE_HPP
#define DNF5_PLUGINS_REPOMANAGE_PLUGIN_REPOMANAGE_HPP


#include "libdnf5/utils/fs/temp.hpp"

#include <dnf5/context.hpp>
#include <libdnf5-cli/session.hpp>


namespace dnf5 {


class RepomanageCommand : public Command {
public:
    explicit RepomanageCommand(Context & context) : Command(context, "repomanage") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void pre_configure() override;
    void configure() override;
    void load_additional_packages() override;
    void run() override;

private:
    std::filesystem::path repo_path;
    libdnf5::OptionNumber<std::int32_t> * keep_count{nullptr};

    enum class Mode { NEW, OLD };
    Mode mode;
    std::unique_ptr<libdnf5::cli::session::BoolOption> old_option{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> new_option{nullptr};

    std::unique_ptr<libdnf5::cli::session::BoolOption> space_option{nullptr};

    std::optional<libdnf5::utils::fs::TempDir> repodata_cache;
    bool repo_with_repodata;
};


}  // namespace dnf5


#endif  // DNF5_PLUGINS_REPOmanage_PLUGIN_REPOMANAGE_HPP
