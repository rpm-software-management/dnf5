// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_COMMANDS_LIST_INFO_HPP
#define DNF5_COMMANDS_LIST_INFO_HPP

#include "list.hpp"

#include <libdnf5-cli/output/package_info_sections.hpp>

namespace dnf5 {


class InfoCommand : public ListCommand {
public:
    explicit InfoCommand(Context & context) : ListCommand(context, "info") {}

private:
    std::unique_ptr<libdnf5::cli::output::PackageListSections> create_output() override;
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_LIST_INFO_HPP
