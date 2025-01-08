#ifndef DNF5_COMMANDS_TEMPLATE_TEMPLATE_HPP
#define DNF5_COMMANDS_TEMPLATE_TEMPLATE_HPP

#include "arguments.hpp"

#include <dnf5/context.hpp>
#include <libdnf5/conf/option_bool.hpp>

#include <memory>
#include <vector>

namespace dnf5 {

// This template aims to explain how to add a new command to dnf5.
// This is the definition of a command called `template` with two options:
// --foo and --bar.
class TemplateCommand : public Command {
public:
    // The command name is set directly in the constructor.
    explicit TemplateCommand(Context & context) : Command(context, "template") {}

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
    // There are two ways to specify options for a command.

    // Option 1: C++ Pointer.
    // Create an OptionBool pointer for the option you need.
    //
    // Using a pure C++ pointer is safe here since the OptionBool class
    // will take care of moving the pointer ownership to the parser, which will
    // be in charge of handling the memory deallocation.
    libdnf5::OptionBool * foo_option{nullptr};

    // Option 2: STL Unique Pointer
    // Create a std::unique_ptr<BarOption>
    //
    // This might be needed in case you need full control over the options and
    // do not want dnf5's parser to handle it.
    // To use a unique_ptr, BarOption has to be defined.
    // A boolean BarOption is defined in the accompanying "arguments.hpp".
    std::unique_ptr<BarOption> bar_option{nullptr};
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_TEMPLATE_TEMPLATE_HPP
