// Copyright (C) 2024 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later

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
