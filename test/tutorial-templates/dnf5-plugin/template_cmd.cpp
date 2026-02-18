// Adjust :lines: in template-command.rst if removing/modifying these comment lines!
// clang-format off
#include "template_cmd.hpp"

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void TemplateCommand::set_parent_command() {
    auto * parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * this_cmd = get_argument_parser_command();
    parent_cmd->register_command(this_cmd);

    auto & group = parent_cmd->get_group("software_management_commands");
    group.register_argument(this_cmd);
}

void TemplateCommand::set_argument_parser() {
    // Context is the main object in dnf5.
    // It contains useful functions and pieces of information necessary to run
    // and interact with dnf5 capabilities.
    //
    // For example, here we need the parser object which is necessary to define
    // commands and options.
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & this_cmd = *get_argument_parser_command();

    // Configure 'template' command in the Parser
    //
    // Set a description that would be displayed in the second column of
    // the main help message.
    this_cmd.set_description("A command that prints its name and arguments' name");

    // Add '--foo' and '--bar' options
    //
    // Option 1: as said in the header file here you should handle the pointer
    // by giving up the ownership to dnf5's parser.
    // Set the default value here.
    foo_option = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(
            std::unique_ptr<libdnf5::OptionBool>(
                new libdnf5::OptionBool(false))));

    // Create an option by giving it a name. It will be shown in the help message.
    auto foo = parser.add_new_named_arg("foo");
    // Set the long name for the option.
    foo->set_long_name("foo");
    // Set the option's description (for the help message).
    foo->set_description("print foo");
    // Set the constant value. This is the value assigned when an option
    // which takes no value arguments appears on the command line.
    foo->set_const_value("true");
    // Link the option to the TemplateCommand's class member.
    foo->link_value(foo_option);

    // Register the '--foo' argument to the 'template' command.
    this_cmd.register_named_arg(foo);

    // Option 2: create a unique_ptr of type BarOption.
    // The long name, description, and other values were set in
    // template.hpp when we derived the BarOption class.
    bar_option = std::make_unique<BarOption>(*this);
}

void TemplateCommand::run() {
    // The behavior of the command is a simple hello world.
    std::cout << "Template Command" << std::endl;

    if (foo_option->get_value()) {
        std::cout << "foo" << std::endl;
    }
    if (bar_option->get_value()) {
        std::cout << "bar" << std::endl;
    }
}

}  // namespace dnf5
