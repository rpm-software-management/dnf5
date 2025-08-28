// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


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
LIBDNF_CLI_API std::ostream & cursor_down(std::ostream & stream);

LIBDNF_CLI_API std::ostream & cursor_hide(std::ostream & stream);
LIBDNF_CLI_API std::ostream & cursor_show(std::ostream & stream);


LIBDNF_CLI_API int get_width();
LIBDNF_CLI_API bool is_interactive();

enum class ColoringEnabled { AUTO, ALWAYS, NEVER };

LIBDNF_CLI_API void coloring_enable(ColoringEnabled);
LIBDNF_CLI_API bool is_coloring_enabled();


}  // namespace libdnf5::cli::tty


#endif  // LIBDNF5_CLI_TTY_HPP
