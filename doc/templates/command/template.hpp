#ifndef DNF5_COMMANDS_TEMPLATE_TEMPLATE_HPP
#define DNF5_COMMANDS_TEMPLATE_TEMPLATE_HPP

#include "arguments.hpp"

#include <dnf5/context.hpp>
#include <libdnf/conf/option_bool.hpp>

#include <memory>
#include <vector>


namespace dnf5 {

// This template aims to explain how to add a new command to dnf5.
// This is the definition of a command called `template` with two options:
// --foo and --bar.
class TemplateCommand : public Command {
public:
    // The command name is set directly in the constructor.
    explicit TemplateCommand(Command & parent) : Command(parent, "template") {}

    // This method needs to be overridden to define the structure of the
    // command such as description, options or sub-commands.
    void set_argument_parser() override;

    // This method needs to be overriden to run the command.
    void run() override;

private:
    // There are two ways to specify options for a command.

    // Option 1: C++ Pointer.
    // Create a OptionBool pointer for the option you need.
    //
    // Using a pure C++ pointer is safe here since the OptionBool class
    // will take care of moving the pointer ownership to the parser, which will
    // be in charge of handling the memory deallocation.
    libdnf5::OptionBool * foo_option{nullptr};

    // Option 2: STL Unique Pointer
    // Create a unique_ptr<BarOption>
    //
    // This might be needed in case you need full control over the options and
    // do not want dnf5's parser to handle it.
    // To use a unique_ptr, BarOption has to be defined (see the file
    // dnf5/commands/arguments.hpp)
    std::unique_ptr<BarOption> bar_option{nullptr};
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_TEMPLATE_TEMPLATE_HPP
