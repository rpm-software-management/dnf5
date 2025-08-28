// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DNF5_COMMANDS_ENVIRONMENT_ARGUMENTS_HPP
#define DNF5_COMMANDS_ENVIRONMENT_ARGUMENTS_HPP


#include <libdnf5-cli/session.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>

namespace dnf5 {


class EnvironmentAvailableOption : public libdnf5::cli::session::BoolOption {
public:
    explicit EnvironmentAvailableOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "available", '\0', _("Show only available environments."), false) {}
};


class EnvironmentInstalledOption : public libdnf5::cli::session::BoolOption {
public:
    explicit EnvironmentInstalledOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "installed", '\0', _("Show only installed environments."), false) {}
};


class EnvironmentSpecArguments : public libdnf5::cli::session::StringArgumentList {
public:
    explicit EnvironmentSpecArguments(libdnf5::cli::session::Command & command)
        : StringArgumentList(command, "environment-spec", _("Pattern matching environment IDs.")) {}
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_ENVIRONMENT_ARGUMENTS_HPP
