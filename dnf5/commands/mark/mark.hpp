/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef DNF5_COMMANDS_MARK_MARK_HPP
#define DNF5_COMMANDS_MARK_MARK_HPP

#include <dnf5/context.hpp>
#include <libdnf/transaction/transaction_item_reason.hpp>

#include <optional>
#include <string>
#include <vector>

namespace dnf5 {

class MarkCommand : public Command {
public:
    explicit MarkCommand(Command & parent) : Command(parent, "mark") {}
    void set_argument_parser() override;
    void register_subcommands() override;
    void pre_configure() override;
};


class MarkUserCommand : public Command {
public:
    explicit MarkUserCommand(Command & parent)
        : MarkUserCommand(parent, "user", libdnf::transaction::TransactionItemReason::USER) {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

protected:
    explicit MarkUserCommand(
        Command & parent, const std::string & name, libdnf::transaction::TransactionItemReason reason)
        : Command(parent, name),
          reason(reason) {}
    std::vector<std::unique_ptr<libdnf::Option>> * pkg_specs{nullptr};
    libdnf::transaction::TransactionItemReason reason;
};


class MarkDependencyCommand : public MarkUserCommand {
public:
    explicit MarkDependencyCommand(Command & parent)
        : MarkUserCommand(parent, "dependency", libdnf::transaction::TransactionItemReason::DEPENDENCY) {}
    void set_argument_parser() override {
        MarkUserCommand::set_argument_parser();
        get_argument_parser_command()->set_description("Mark package as a dependency");
    }
};


class MarkWeakDependencyCommand : public MarkUserCommand {
public:
    explicit MarkWeakDependencyCommand(Command & parent)
        : MarkUserCommand(parent, "weak", libdnf::transaction::TransactionItemReason::WEAK_DEPENDENCY) {}
    void set_argument_parser() override {
        MarkUserCommand::set_argument_parser();
        get_argument_parser_command()->set_description("Mark package as a weak dependency");
    }
};


class MarkGroupCommand : public MarkUserCommand {
public:
    explicit MarkGroupCommand(Command & parent)
        : MarkUserCommand(parent, "group", libdnf::transaction::TransactionItemReason::GROUP) {}
    void set_argument_parser() override;
    void run() override;

protected:
    std::string group_id;
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_MARK_MARK_HPP
