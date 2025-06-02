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


#ifndef DNF5_COMMANDS_DOWNLOAD_DOWNLOAD_HPP
#define DNF5_COMMANDS_DOWNLOAD_DOWNLOAD_HPP

#include <dnf5/context.hpp>
#include <libdnf5/conf/option.hpp>

#include <memory>
#include <set>
#include <vector>


namespace dnf5 {


class DownloadCommand : public Command {
public:
    explicit DownloadCommand(Context & context) : Command(context, "download") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    std::set<std::string> urlprotocol_valid_options;
    std::set<std::string> urlprotocol_option;
    std::set<std::string> arch_option;
    libdnf5::OptionBool * resolve_option{nullptr};
    libdnf5::OptionBool * alldeps_option{nullptr};
    libdnf5::OptionBool * url_option{nullptr};
    libdnf5::OptionBool * allmirrors_option{nullptr};
    libdnf5::OptionBool * srpm_option{nullptr};

    std::vector<std::string> from_repos;

    std::vector<std::unique_ptr<libdnf5::Option>> * patterns_to_download_options{nullptr};
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_DOWNLOAD_DOWNLOAD_HPP
