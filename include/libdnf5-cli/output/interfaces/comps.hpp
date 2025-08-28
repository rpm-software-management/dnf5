// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_INTERFACES_COMPS_HPP
#define LIBDNF5_CLI_OUTPUT_INTERFACES_COMPS_HPP

#include <libdnf5/comps/group/package_type.hpp>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace libdnf5::cli::output {

class IGroupPackage {
public:
    virtual ~IGroupPackage() = default;

    virtual std::string get_name() const = 0;
    virtual libdnf5::comps::PackageType get_type() const = 0;
};


class IGroup {
public:
    virtual ~IGroup() = default;

    virtual std::string get_groupid() const = 0;
    virtual std::string get_name() const = 0;
    virtual std::string get_description() const = 0;
    virtual std::string get_order() const = 0;
    virtual int get_order_int() const = 0;
    virtual std::string get_langonly() const = 0;
    virtual bool get_uservisible() const = 0;
    virtual std::vector<std::unique_ptr<IGroupPackage>> get_packages() = 0;
    virtual std::set<std::string> get_repos() const = 0;
    virtual bool get_installed() const = 0;
};


class IEnvironment {
public:
    virtual ~IEnvironment() = default;

    virtual std::string get_environmentid() const = 0;
    virtual std::string get_name() const = 0;
    virtual std::string get_description() const = 0;
    virtual std::string get_order() const = 0;
    virtual int get_order_int() const = 0;
    virtual std::vector<std::string> get_groups() = 0;
    virtual std::vector<std::string> get_optional_groups() = 0;
    virtual std::set<std::string> get_repos() const = 0;
    virtual bool get_installed() const = 0;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_INTERFACES_COMPS_HPP
