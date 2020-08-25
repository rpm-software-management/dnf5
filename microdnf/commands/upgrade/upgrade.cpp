/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of microdnf: https://github.com/rpm-software-management/libdnf/

Microdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Microdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with microdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "upgrade.hpp"

#include "../../context.hpp"

#include <libdnf/conf/option_string.hpp>
#include <libdnf/rpm/package.hpp>
#include <libdnf/rpm/package_set.hpp>
#include <libdnf/rpm/solv_query.hpp>
#include <libdnf/rpm/transaction.hpp>
#include <libsmartcols/libsmartcols.h>

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace microdnf {

void CmdUpgrade::set_argument_parser(Context & ctx) {
    patterns_to_upgrade_options = ctx.arg_parser.add_new_values();
    auto keys = ctx.arg_parser.add_new_positional_arg(
        "keys_to_match",
        ArgumentParser::PositionalArg::UNLIMITED,
        ctx.arg_parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_to_upgrade_options);
    keys->set_short_description("List of keys to match");

    auto upgrade = ctx.arg_parser.add_new_command("upgrade");
    upgrade->set_short_description("upgrade a package or packages on your system");
    upgrade->set_description("");
    upgrade->named_args_help_header = "Optional arguments:";
    upgrade->positional_args_help_header = "Positional arguments:";
    upgrade->parse_hook = [this, &ctx](
                                [[maybe_unused]] ArgumentParser::Argument * arg,
                                [[maybe_unused]] const char * option,
                                [[maybe_unused]] int argc,
                                [[maybe_unused]] const char * const argv[]) {
        ctx.select_command(this);
        return true;
    };

    upgrade->add_positional_arg(keys);

    ctx.arg_parser.get_root_command()->add_command(upgrade);
}

void CmdUpgrade::configure([[maybe_unused]] Context & ctx) {}

class TransCB : public libdnf::rpm::TransactionCB {
    void install_start(const libdnf::rpm::TransactionItem * /*item*/, const libdnf::rpm::RpmHeader & header, uint64_t total) override {
        std::cout << "Start: " << header.get_full_nevra() << "Size: " << total << std::endl;
    }
    void install_progress(const libdnf::rpm::TransactionItem * /*item*/, const libdnf::rpm::RpmHeader & header, uint64_t amount, uint64_t total) override {
        std::cout << "Progress: " << header.get_full_nevra() << " " << amount << '/' << total << std::endl;
    }
};

void CmdUpgrade::run(Context & ctx) {
    using LoadFlags = libdnf::rpm::SolvSack::LoadRepoFlags;
    auto & solv_sack = ctx.base.get_rpm_solv_sack();

    // To search in the system repository (installed packages)
    // Creates system repository in the repo_sack and loads it into rpm::SolvSack.
    //solv_sack.create_system_repo(false);

    // To search in available repositories (available packages)
    auto enabled_repos = ctx.base.get_rpm_repo_sack().new_query().ifilter_enabled(true);
    for (auto & repo : enabled_repos.get_data()) {
        ctx.load_rpm_repo(*repo.get());
    }

    for (auto & repo : enabled_repos.get_data()) {
        solv_sack.load_repo(
            *repo.get(),
            LoadFlags::USE_FILELISTS | LoadFlags::USE_PRESTO | LoadFlags::USE_UPDATEINFO | LoadFlags::USE_OTHER);
    }

    libdnf::rpm::PackageSet result_pset(&solv_sack);
    libdnf::rpm::SolvQuery full_solv_query(&solv_sack);
    for (auto & pattern : *patterns_to_upgrade_options) {
        libdnf::rpm::SolvQuery solv_query(full_solv_query);
        solv_query.resolve_pkg_spec(dynamic_cast<libdnf::OptionString *>(pattern.get())->get_value(), true, true, true, true, true, {});
        result_pset |= solv_query.get_package_set();
    }

    for (auto package : result_pset) {
        std::cout << package.get_full_nevra() << '\n';
        std::cout << package.get_url() << '\n';
        std::cout << package.get_baseurl() << '\n';
        std::cout << package.get_location() << '\n';
        //auto repo = package.get_repo();
        //fd = open();
        //repo.download_url(url, fd);
        //close(fd);
    }

    // download packages
    std::vector<libdnf::rpm::PackageTarget *> targets;
    try {
        for (auto package : result_pset) {
            auto repo = package.get_repo();
            auto destination = fs::path(repo->get_cachedir()) / "packages";
            auto checksum = package.get_checksum();
            auto pkg_target = new libdnf::rpm::PackageTarget(repo, package.get_location().c_str(), destination.c_str(),
                                    static_cast<int>(checksum.get_type()), checksum.get_checksum().c_str(),
                                    static_cast<int64_t>(package.get_download_size()), package.get_baseurl().empty() ? nullptr : package.get_baseurl().c_str(), true,
                                    0, 0, nullptr);
            targets.push_back(pkg_target);
        }
        std::cout << "Start packages download" << std::endl;
        try {
            libdnf::rpm::PackageTarget::download_packages(targets, true);
        } catch (const std::runtime_error & ex) {
            std::cout << "Exception: " << ex.what() << std::endl;
        }
        std::cout << "Done packages download" << std::endl;
    } catch (...) {
        for (auto target : targets) {
            delete target;
        }
        throw;
    }
    for (auto target : targets) {
        delete target;
    }

    std::vector<std::unique_ptr<libdnf::rpm::TransactionItem>> transaction_items;
    auto ts = libdnf::rpm::Transaction(ctx.base);
    for (auto package : result_pset) {
        auto item = std::make_unique<libdnf::rpm::TransactionItem>(package);
        auto item_ptr = item.get();
        transaction_items.push_back(std::move(item));
        ts.upgrade(*item_ptr);
    }
    TransCB trans_cb;
    ts.register_cb(&trans_cb);
    ts.run();
    ts.register_cb(nullptr);
}

}  // namespace microdnf
