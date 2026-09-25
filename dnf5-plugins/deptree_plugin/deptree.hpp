// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DNF5_PLUGINS_DEPTREE_PLUGIN_DEPTREE_HPP
#define DNF5_PLUGINS_DEPTREE_PLUGIN_DEPTREE_HPP

#include <dnf5/context.hpp>
#include <libdnf5-cli/session.hpp>
#include <libdnf5/conf/option_number.hpp>
#include <libdnf5/conf/option_string_list.hpp>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace dnf5 {

class DeptreeCommand : public Command {
public:
    explicit DeptreeCommand(Context & context) : Command(context, "deptree") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    std::vector<std::string> pkg_specs{};
    libdnf5::OptionStringSet * dependency_types_option{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> reverse{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> flat{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> show_duplicates{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> show_requires{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> installed{nullptr};
    std::set<std::string> arches{};
    libdnf5::OptionNumber<std::int32_t> * depth_option{nullptr};
};

}  // namespace dnf5

#endif  // DNF5_PLUGINS_DEPTREE_PLUGIN_DEPTREE_HPP
