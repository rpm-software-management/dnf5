// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_COMMANDS_LIST_LIST_HPP
#define DNF5_COMMANDS_LIST_LIST_HPP

#include <dnf5/context.hpp>
#include <libdnf5-cli/output/package_list_sections.hpp>
#include <libdnf5-cli/session.hpp>
#include <libdnf5/rpm/package_set.hpp>

#include <memory>
#include <vector>


namespace dnf5 {


class ListCommand : public Command {
public:
    explicit ListCommand(Context & context) : Command(context, "list") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

protected:
    explicit ListCommand(Context & context, const std::string & name) : Command(context, name) {}

private:
    virtual std::unique_ptr<libdnf5::cli::output::PackageListSections> create_output();

    enum class PkgNarrow { ALL, INSTALLED, AVAILABLE, EXTRAS, OBSOLETES, RECENT, UPGRADES, AUTOREMOVE };

    std::vector<std::string> pkg_specs;

    // options to narrow the output packages
    PkgNarrow pkg_narrow{PkgNarrow::ALL};
    std::unique_ptr<libdnf5::cli::session::BoolOption> installed{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> available{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> extras{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> obsoletes{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> recent{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> upgrades{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> autoremove{nullptr};

    // show all NEVRAs, not only the latest one
    std::unique_ptr<libdnf5::cli::session::BoolOption> show_duplicates{nullptr};

    std::vector<std::string> installed_from_repos;
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_LIST_LIST_HPP
