#include "template.hpp"

#include <iostream>

namespace dnf5 {

using namespace libdnf::cli;

void TemplateCommand::set_argument_parser() {
    // Context is the main object in dnf5.
    // It contains useful functions and pieces of information necessary to run
    // and interact with dnf5 capabilities.
    //
    // For example, here we need the parser object which is necessary to define
    // commands and options.
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();

    // Add template command to the Parser
    //
    // Set a description that would be displayed in the second column of the
    // help message would also add the command to the parser
    cmd.set_short_description("A command that prints its name and arguments' name");

    // Add foo and bar options
    //
    // Option 1: as said in the header file here you should handle the pointer
    // by giving up the ownership to dnf5's parser.
    // Set the default value here.
    foo_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    // Create an option by giving it a name. It will be shown in the help message.
    // Set long name, description and constant value.
    // Link the option to the TemplateCommand's class member.
    auto foo = parser.add_new_named_arg("foo");
    foo->set_long_name("foo");
    foo->set_short_description("print foo");
    foo->set_const_value("true");
    foo->link_value(foo_option);

    // Register the argument to the command template.
    cmd.register_named_arg(foo);

    // Option 2: create a unique_ptr of type BarOption.
    // The long name, description, and other values were set in
    // dnf5/command/arguments.hpp
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
