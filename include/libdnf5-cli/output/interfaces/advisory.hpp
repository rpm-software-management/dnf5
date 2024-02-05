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


#ifndef LIBDNF5_CLI_OUTPUT_INTERFACES_ADVISORY_HPP
#define LIBDNF5_CLI_OUTPUT_INTERFACES_ADVISORY_HPP

#include <memory>
#include <string>
#include <vector>

namespace libdnf5::cli::output {

class IAdvisory;


class IAdvisoryPackage {
public:
    virtual ~IAdvisoryPackage() = default;

    virtual std::string get_name() const = 0;
    virtual std::string get_epoch() const = 0;
    virtual std::string get_version() const = 0;
    virtual std::string get_release() const = 0;
    virtual std::string get_arch() const = 0;
    virtual std::string get_nevra() const = 0;
    virtual std::unique_ptr<IAdvisory> get_advisory() const = 0;
};


class IAdvisoryReference {
public:
    virtual ~IAdvisoryReference() = default;

    virtual std::string get_id() const = 0;
    virtual std::string get_type() const = 0;
    virtual const char * get_type_cstring() const = 0;
    virtual std::string get_title() const = 0;
    virtual std::string get_url() const = 0;
};


class IAdvisoryModule {
public:
    virtual ~IAdvisoryModule() = default;

    virtual std::string get_name() const = 0;
    virtual std::string get_stream() const = 0;
    virtual std::string get_version() const = 0;
    virtual std::string get_context() const = 0;
    virtual std::string get_arch() const = 0;
    virtual std::string get_nsvca() const = 0;
    virtual std::unique_ptr<IAdvisory> get_advisory() const = 0;
};


class IAdvisoryCollection {
public:
    virtual ~IAdvisoryCollection() = default;

    virtual std::vector<std::unique_ptr<IAdvisoryPackage>> get_packages() = 0;
    virtual std::vector<std::unique_ptr<IAdvisoryModule>> get_modules() = 0;
};


class IAdvisory {
public:
    virtual ~IAdvisory() = default;

    virtual std::string get_name() const = 0;
    virtual std::string get_severity() const = 0;
    virtual std::string get_type() const = 0;
    virtual unsigned long long get_buildtime() const = 0;
    virtual std::string get_vendor() const = 0;
    virtual std::string get_description() const = 0;
    virtual std::string get_title() const = 0;
    virtual std::string get_status() const = 0;
    virtual std::string get_rights() const = 0;
    virtual std::string get_message() const = 0;
    virtual std::vector<std::unique_ptr<IAdvisoryReference>> get_references() const = 0;
    virtual std::vector<std::unique_ptr<IAdvisoryCollection>> get_collections() const = 0;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_INTERFACES_ADVISORY_HPP
