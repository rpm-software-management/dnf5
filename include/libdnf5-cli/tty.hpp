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


#ifndef LIBDNF5_CLI_TTY_HPP
#define LIBDNF5_CLI_TTY_HPP

#include "libdnf5-cli/defs.h"

#include <iostream>


namespace libdnf5::cli::tty {


LIBDNF_CLI_API std::ostream & reset(std::ostream & stream);

LIBDNF_CLI_API std::ostream & bold(std::ostream & stream);
LIBDNF_CLI_API std::ostream & underline(std::ostream & stream);
LIBDNF_CLI_API std::ostream & blink(std::ostream & stream);

LIBDNF_CLI_API std::ostream & black(std::ostream & stream);
LIBDNF_CLI_API std::ostream & red(std::ostream & stream);
LIBDNF_CLI_API std::ostream & green(std::ostream & stream);
LIBDNF_CLI_API std::ostream & yellow(std::ostream & stream);
LIBDNF_CLI_API std::ostream & blue(std::ostream & stream);
LIBDNF_CLI_API std::ostream & magenta(std::ostream & stream);
LIBDNF_CLI_API std::ostream & cyan(std::ostream & stream);
LIBDNF_CLI_API std::ostream & white(std::ostream & stream);

LIBDNF_CLI_API std::ostream & clear_line(std::ostream & stream);
LIBDNF_CLI_API std::ostream & cursor_up(std::ostream & stream);

LIBDNF_CLI_API std::ostream & cursor_hide(std::ostream & stream);
LIBDNF_CLI_API std::ostream & cursor_show(std::ostream & stream);


LIBDNF_CLI_API int get_width();
LIBDNF_CLI_API bool is_interactive();


}  // namespace libdnf5::cli::tty


#endif  // LIBDNF5_CLI_TTY_HPP
