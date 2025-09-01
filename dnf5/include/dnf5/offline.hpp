// Copyright (C) 2024 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef DNF5_OFFLINE_HPP
#define DNF5_OFFLINE_HPP

#include "context.hpp"
#include "defs.h"

#include <string>

namespace dnf5::offline {

DNF_API void log_status(
    Context & context,
    const std::string & message,
    const std::string & message_id,
    const std::string & system_releasever,
    const std::string & target_releasever);

}  // namespace dnf5::offline

#endif  // DNF5_OFFLINE_HPP
