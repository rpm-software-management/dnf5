// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


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
LIBDNF_CLI_API std::ostream & clear_to_end(std::ostream & stream);
LIBDNF_CLI_API std::ostream & cursor_up(std::ostream & stream);
LIBDNF_CLI_API std::ostream & cursor_down(std::ostream & stream);

LIBDNF_CLI_API std::ostream & cursor_hide(std::ostream & stream);
LIBDNF_CLI_API std::ostream & cursor_show(std::ostream & stream);


/// State of the terminal taskbar/progress indicator, following the ConEmu
/// `OSC 9;4` de facto protocol (also supported by Konsole, Windows Terminal,
/// foot, WezTerm, kitty, ...). Terminals that don't understand it ignore it.
/// Spec: https://learn.microsoft.com/en-us/windows/terminal/tutorials/progress-bar-sequences
/// Origin: https://conemu.github.io/en/AnsiEscapeCodes.html#ConEmu_specific_OSC
enum class TaskbarProgressState {
    CLEAR = 0,          // remove the indicator
    NORMAL = 1,         // normal progress, uses percent
    ERROR = 2,          // error state (red), uses percent
    INDETERMINATE = 3,  // busy/indeterminate (percent ignored)
    WARNING = 4,        // paused/warning (yellow)
};

/// Emit an `OSC 9;4` escape sequence reporting overall progress to the terminal,
/// which some terminals render on the tab/card/taskbar entry.
/// @param stream   Stream to write the sequence to.
/// @param state    Indicator state.
/// @param percent  Progress percentage, clamped to 0-100 (ignored for
///                 CLEAR and INDETERMINATE states).
LIBDNF_CLI_API std::ostream & set_taskbar_progress(std::ostream & stream, TaskbarProgressState state, int percent = 0);


LIBDNF_CLI_API int get_width();
LIBDNF_CLI_API bool is_interactive();

enum class ColoringEnabled { AUTO, ALWAYS, NEVER };

LIBDNF_CLI_API void coloring_enable(ColoringEnabled);
LIBDNF_CLI_API bool is_coloring_enabled();


}  // namespace libdnf5::cli::tty


#endif  // LIBDNF5_CLI_TTY_HPP
