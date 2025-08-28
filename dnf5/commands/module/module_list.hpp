// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_MODULE_MODULE_LIST_HPP
#define DNF5_COMMANDS_MODULE_MODULE_LIST_HPP

#include "arguments.hpp"

#include <dnf5/context.hpp>
#include <libdnf5/module/module_query.hpp>

namespace dnf5 {

class ModuleListCommand : public Command {
public:
    explicit ModuleListCommand(Context & context) : Command(context, "list") {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

protected:
    ModuleListCommand(Context & context, const std::string & name) : Command(context, name) {}

private:
    std::unique_ptr<ModuleEnabledOption> enabled{nullptr};
    std::unique_ptr<ModuleDisabledOption> disabled{nullptr};
    std::unique_ptr<ModuleSpecArguments> module_specs{nullptr};

    virtual void print(const libdnf5::module::ModuleQuery & query);
    virtual void print_hint();
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_MODULE_MODULE_LIST_HPP
