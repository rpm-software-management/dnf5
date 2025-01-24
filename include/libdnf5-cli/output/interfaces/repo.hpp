/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF5_CLI_OUTPUT_INTERFACES_REPO_HPP
#define LIBDNF5_CLI_OUTPUT_INTERFACES_REPO_HPP

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace libdnf5::cli::output {

class IRepo {
public:
    virtual ~IRepo() = default;

    virtual std::string get_id() const = 0;
    virtual std::string get_name() const = 0;
    virtual bool is_enabled() const = 0;
};


class IRepoInfo {
public:
    virtual ~IRepoInfo() = default;

    virtual std::string get_id() const = 0;
    virtual std::string get_name() const = 0;
    virtual std::string get_type() const = 0;
    virtual bool is_enabled() const = 0;
    virtual int get_priority() const = 0;
    virtual int get_cost() const = 0;
    virtual std::vector<std::string> get_baseurl() const = 0;
    virtual std::string get_metalink() const = 0;
    virtual std::string get_mirrorlist() const = 0;
    virtual int get_metadata_expire() const = 0;
    virtual std::vector<std::string> get_excludepkgs() const = 0;
    virtual std::vector<std::string> get_includepkgs() const = 0;
    virtual bool get_skip_if_unavailable() const = 0;
    virtual std::vector<std::string> get_gpgkey() const = 0;
    virtual bool get_pkg_gpgcheck() const = 0;
    /// @deprecated Use get_pkg_gpgcheck() const
    [[deprecated("Use get_pkg_gpgcheck() const")]]
    virtual bool get_gpgcheck() const = 0;
    virtual bool get_repo_gpgcheck() const = 0;
    virtual std::string get_repo_file_path() const = 0;
    virtual std::string get_revision() const = 0;
    virtual std::vector<std::string> get_content_tags() const = 0;
    virtual std::vector<std::pair<std::string, std::string>> get_distro_tags() const = 0;
    virtual int64_t get_timestamp() const = 0;
    virtual int get_max_timestamp() const = 0;
    virtual uint64_t get_size() const = 0;
    virtual uint64_t get_pkgs() const = 0;
    virtual uint64_t get_available_pkgs() const = 0;
    virtual std::vector<std::string> get_mirrors() const = 0;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_INTERFACES_REPO_HPP
