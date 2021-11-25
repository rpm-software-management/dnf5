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


#ifndef LIBDNF_CLI_TTY_HPP
#define LIBDNF_CLI_TTY_HPP


#include <iostream>


namespace libdnf::cli::tty {


std::ostream & reset(std::ostream & stream);

std::ostream & bold(std::ostream & stream);
std::ostream & underline(std::ostream & stream);
std::ostream & blink(std::ostream & stream);

std::ostream & black(std::ostream & stream);
std::ostream & red(std::ostream & stream);
std::ostream & green(std::ostream & stream);
std::ostream & yellow(std::ostream & stream);
std::ostream & blue(std::ostream & stream);
std::ostream & magenta(std::ostream & stream);
std::ostream & cyan(std::ostream & stream);
std::ostream & white(std::ostream & stream);

std::ostream & clear_line(std::ostream & stream);
std::ostream & cursor_up(std::ostream & stream);

std::ostream & cursor_hide(std::ostream & stream);
std::ostream & cursor_show(std::ostream & stream);


int get_width();
bool is_interactive();


}  // namespace libdnf::cli::tty


#endif  // LIBDNF_CLI_TTY_HPP
