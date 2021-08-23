/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include <libdnf-cli/session.hpp>


namespace libdnf::cli::session {


Command::Command(Session & session, const std::string & program_name) : session{session} {
    // create a new root command; owned by the arg_parser
    argument_parser_command = session.get_argument_parser().add_new_command(program_name);

    // register command as root command
    session.get_argument_parser().set_root_command(argument_parser_command);

    // set the created root command as the selected
    // the value will get overwritten with a subcommand during parsing the arguments
    session.set_selected_command(this);
}


Command::Command(Command & parent, const std::string & name) : session{parent.session}, parent_command{&parent} {
    // create a new command (owned by the arg_parser)
    argument_parser_command = session.get_argument_parser().add_new_command(name);

    // set the command as selected when parsed
    argument_parser_command->set_parse_hook_func([this](
                                [[maybe_unused]] ArgumentParser::Argument * arg,
                                [[maybe_unused]] const char * option,
                                [[maybe_unused]] int argc,
                                [[maybe_unused]] const char * const argv[]) {

        // set the selected command only if a subcommand hasn't set it already
        auto & session = this->get_session();
        auto * selected_command = session.get_selected_command();
        if (!selected_command || selected_command == session.get_root_command()) {
            session.set_selected_command(this);
        }
        return true;
    });
}


void Command::register_subcommand(std::unique_ptr<Command> subcommand, libdnf::cli::ArgumentParser::Group * group) {
    get_argument_parser_command()->register_command(subcommand->get_argument_parser_command());
    if (group) {
        group->register_argument(subcommand->get_argument_parser_command());
    }
    subcommands.push_back(std::move(subcommand));
}


}  // namespace libdnf::cli::session
