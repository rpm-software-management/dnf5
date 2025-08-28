// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


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
