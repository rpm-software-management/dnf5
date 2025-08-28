// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "copr.hpp"
#include "copr_repo.hpp"

#include <iostream>

namespace dnf5 {


class RepoListCB : public CoprRepoCallback {
private:
    std::string hostname;

public:
    explicit RepoListCB(const std::string & host) : hostname(host) {};
    dnf5::CoprRepoCallback list = [&](dnf5::CoprRepo & cr) {
        if (!hostname.empty() && cr.get_id().rfind(hostname + "/", 0) != 0)
            return;
        std::cout << cr.get_id();
        if (cr.has_external_deps())
            std::cout << " [eternal_deps]";
        if (cr.is_multilib())
            std::cout << " [multilib]";
        if (!cr.is_enabled())
            std::cout << " (disabled)";
        std::cout << std::endl;
    };
};


void CoprListCommand::set_argument_parser() {
    auto & cmd = *this->get_argument_parser_command();
    auto desc = _("list Copr repositories");
    cmd.set_description(desc);
    cmd.set_long_description(desc);

    // this->installed = std::make_unique<libdnf5::cli::session::BoolOption>(*this, "installed", 'i', "installed", false);

    // --installed           List all installed Copr repositories (default)
    // --enabled             List enabled Copr repositories
    // --disabled            List disabled Copr repositories
    // --available-by-user NAME
    //                       List available Copr repositories by user NAME
    // --hub HUB             Specify an instance of Copr to work with
}


void CoprListCommand::run() {
    auto & base = get_context().get_base();
    std::unique_ptr<dnf5::CoprConfig> config = std::make_unique<dnf5::CoprConfig>(base);
    // empty string if no --hub is specified
    auto hostname = copr_cmd()->hub();
    if (!hostname.empty())
        // try to resolve hubpec => hostname
        hostname = config->get_hub_hostname(hostname);
    auto list = RepoListCB(hostname);
    installed_copr_repositories(base, list.list);
}

}  // namespace dnf5
