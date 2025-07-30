// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DNF5_PLUGINS_MANIFEST_PLUGIN_MANIFEST_HPP
#define DNF5_PLUGINS_MANIFEST_PLUGIN_MANIFEST_HPP

#include <dnf5/context.hpp>
#include <libdnf5/conf/option_bool.hpp>

#include <memory>
#include <vector>

namespace dnf5 {

class ManifestCommand : public Command {
public:
    // The command name is set directly in the constructor.
    explicit ManifestCommand(Context & context) : Command(context, "manifest") {}

    // This method needs to be overridden to register the command with
    // the dnf5 command-line parser, and place the command in one of
    // the standard command groups.
    //
    // Every top-level command will have a set_parent_command method.
    // Subcommands will not, their parent command's register_subcommands()
    // method is used to register them.
    void set_parent_command() override;

    // This method needs to be overridden to define the structure of the
    // command such as description and options.
    void set_argument_parser() override;

    // This method MAY be overridden in a top-level command which has
    // subcommands, to register them using register_subcommand().
    // void register_subcommands() override;

    // This method needs to be overridden to run the command.
    void run() override;

    // This method MAY be overridden to perform any configuration
    // needed by the command.
    // void configure() override;

    // This method MAY be overridden to perform any pre-configuration
    // needed by the command.
    // void pre_configure() override;

    // Not every command will need a pre_configure or configure method,
    // and few will need both.
    //
    // A common pattern in commands that have subcommands is to define
    // the top-level command's pre_configure as containing only
    // throw_missing_command(). This ensures that using the command
    // without one of its subcommands will be treated as an error.

private:
    libdnf5::OptionBool * foo_option{nullptr};
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_MANIFEST_MANIFEST_HPP
