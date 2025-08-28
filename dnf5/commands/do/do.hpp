// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_DO_DO_HPP
#define DNF5_COMMANDS_DO_DO_HPP

#include "../advisory_shared.hpp"

#include <dnf5/context.hpp>
#include <dnf5/shared_options.hpp>
#include <libdnf5/rpm/package.hpp>

#include <memory>
#include <vector>

namespace dnf5 {

class DoCommand final : public Command {
public:
    explicit DoCommand(Context & context) : Command(context, "do") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    enum class Action { UNDEFINED, INSTALL, REMOVE, UPGRADE, DOWNGRADE, REINSTALL } action{Action::UNDEFINED};

    enum class ItemType { AUTO, PACKAGE, GROUP, ENVIRONMENT } item_type{ItemType::AUTO};

    std::vector<std::string> installed_from_repos;
    std::vector<std::string> from_repos;

    std::string group_spec(ItemType type, const std::string & spec, libdnf5::GoalJobSettings & settings);

    std::unique_ptr<AllowErasingOption> allow_erasing;

    std::unique_ptr<AdvisoryOption> advisory_name;
    std::unique_ptr<SecurityOption> advisory_security;
    std::unique_ptr<BugfixOption> advisory_bugfix;
    std::unique_ptr<EnhancementOption> advisory_enhancement;
    std::unique_ptr<NewpackageOption> advisory_newpackage;
    std::unique_ptr<AdvisorySeverityOption> advisory_severity;
    std::unique_ptr<BzOption> advisory_bz;
    std::unique_ptr<CveOption> advisory_cve;

    struct ItemSpec {
        ItemType type;
        std::string spec;
        libdnf5::GoalJobSettings settings;
    };

    std::vector<ItemSpec> install_items;
    std::vector<ItemSpec> remove_items;
    std::vector<ItemSpec> upgrade_items;
    std::vector<ItemSpec> downgrade_items;
    std::vector<ItemSpec> reinstall_items;
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_DO_DO_HPP
