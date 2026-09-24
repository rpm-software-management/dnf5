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

#ifndef DNF5_CMDLINE_ALIASES_HPP
#define DNF5_CMDLINE_ALIASES_HPP

#include "dnf5/context.hpp"

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace dnf5 {

/// dnf4 compatibility: one 'command_flag' alias entry. The key of the table
/// is (command id path, flag including the leading dashes). Entries are
/// filled by the alias loader and applied by apply_command_flag_aliases().
struct CommandFlagAlias {
    /// words of the attached command path; empty when reject_message is set
    std::vector<std::string> words;
    /// options always passed after the attached command
    std::vector<std::string> attached_flags;
    /// template applied to each following positional; ${} is the value
    std::string positional_template;
    /// prefix rewrite map for following dashed tokens: prefix -> template
    std::vector<std::pair<std::string, std::string>> token_prefix_maps;
    /// when the flag appears together with this companion flag, the
    /// companion is consumed too
    std::string companion_flag;
    /// when true: following bare positionals are dropped instead of copied;
    /// dropped_note is printed per drop
    bool drop_bare_positionals = false;
    std::string dropped_note;
    /// when true: the value handling also covers tokens between the command
    /// word and the trigger flag
    bool leading_positionals = false;
    /// when set: the rewrite only applies if every following value ends with
    /// this suffix; otherwise gate_reject_message is printed if set and the
    /// arguments are left untouched
    std::string positional_suffix_gate;
    std::string gate_reject_message;
    /// when set: matching prints this message and no rewrite happens
    std::string reject_message;
    /// when several tokens on one line match entries, the highest precedence
    /// executes; ties go to the leftmost token
    int precedence = 0;
};

using CommandFlagAliasTable = std::map<std::pair<std::string, std::string>, CommandFlagAlias>;

/// Command line rewritten by a 'command_flag' alias. The argv pointers point
/// into storage, so the object must outlive the parsing.
struct RewrittenArgv {
    std::vector<std::string> storage;
    std::vector<const char *> argv;
};

/// Creates groups and aliases of command line arguments as defined in configuration files.
/// 'command_flag' alias entries are recorded into the given table instead of
/// being registered with the argument parser.
void load_cmdline_aliases(
    Context & context,
    const std::filesystem::path & config_dir_path,
    const std::string & locale_name,
    CommandFlagAliasTable & command_flag_aliases);

/// Applies 'command_flag' aliases: a dnf4 flag that selects a dnf5 subcommand
/// is translated, with optional rewriting of the values that follow it,
/// before parsing. Returns std::nullopt when no entry matched or the matched
/// entry declined the rewrite; parse the original arguments in that case.
std::optional<RewrittenArgv> apply_command_flag_aliases(
    Context & context, const CommandFlagAliasTable & command_flag_aliases, int argc, const char * const * argv);

}  // namespace dnf5

#endif
