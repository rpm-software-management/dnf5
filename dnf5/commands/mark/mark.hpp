// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#ifndef DNF5_COMMANDS_MARK_MARK_HPP
#define DNF5_COMMANDS_MARK_MARK_HPP

#include <dnf5/context.hpp>
#include <libdnf5/transaction/transaction_item_reason.hpp>

#include <optional>
#include <string>
#include <vector>

namespace dnf5 {

class MarkCommand : public Command {
public:
    explicit MarkCommand(Context & context) : Command(context, "mark") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void register_subcommands() override;
    void pre_configure() override;
};


class MarkUserCommand : public Command {
public:
    explicit MarkUserCommand(Context & context)
        : MarkUserCommand(context, "user", libdnf5::transaction::TransactionItemReason::USER) {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

protected:
    explicit MarkUserCommand(
        Context & context, const std::string & name, libdnf5::transaction::TransactionItemReason reason)
        : Command(context, name),
          reason(reason) {}
    std::vector<std::unique_ptr<libdnf5::Option>> * pkg_specs{nullptr};
    libdnf5::transaction::TransactionItemReason reason;
};


class MarkDependencyCommand : public MarkUserCommand {
public:
    explicit MarkDependencyCommand(Context & context)
        : MarkUserCommand(context, "dependency", libdnf5::transaction::TransactionItemReason::DEPENDENCY) {}
    void set_argument_parser() override {
        MarkUserCommand::set_argument_parser();
        get_argument_parser_command()->set_description("Mark package as a dependency");
    }
};


class MarkWeakDependencyCommand : public MarkUserCommand {
public:
    explicit MarkWeakDependencyCommand(Context & context)
        : MarkUserCommand(context, "weak", libdnf5::transaction::TransactionItemReason::WEAK_DEPENDENCY) {}
    void set_argument_parser() override {
        MarkUserCommand::set_argument_parser();
        get_argument_parser_command()->set_description("Mark package as a weak dependency");
    }
};


class MarkGroupCommand : public MarkUserCommand {
public:
    explicit MarkGroupCommand(Context & context)
        : MarkUserCommand(context, "group", libdnf5::transaction::TransactionItemReason::GROUP) {}
    void set_argument_parser() override;
    void run() override;

protected:
    std::string group_id;
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_MARK_MARK_HPP
