/*
Copyright Contributors to the dnf5 project.

This file is part of dnf5: https://github.com/rpm-software-management/dnf5/

Dnf5 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnf5 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnf5.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DNF5_SIGNAL_HANDLERS_HPP
#define DNF5_SIGNAL_HANDLERS_HPP

namespace dnf5 {

// Register SIGINT and SIGTERM handlers
void install_signal_handlers();

}  // namespace dnf5

#endif  // DNF5_SIGNAL_HANDLERS_HPP
