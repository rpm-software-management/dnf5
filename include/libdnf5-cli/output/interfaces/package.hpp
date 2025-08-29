// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_INTERFACES_PACKAGE_HPP
#define LIBDNF5_CLI_OUTPUT_INTERFACES_PACKAGE_HPP

#include <libdnf5/transaction/transaction_item_reason.hpp>

#include <string>
#include <vector>

namespace libdnf5::cli::output {

class IPackage {
public:
    virtual ~IPackage() = default;

    virtual std::string get_name() const = 0;
    virtual std::string get_epoch() const = 0;
    virtual std::string get_version() const = 0;
    virtual std::string get_release() const = 0;
    virtual std::string get_arch() const = 0;
    virtual std::string get_evr() const = 0;
    virtual std::string get_nevra() const = 0;
    virtual std::string get_full_nevra() const = 0;
    virtual std::string get_na() const = 0;
    virtual unsigned long long get_download_size() const = 0;
    virtual unsigned long long get_install_size() const = 0;
    virtual std::string get_license() const = 0;
    virtual std::string get_sourcerpm() const = 0;
    virtual std::string get_vendor() const = 0;
    virtual std::string get_url() const = 0;
    virtual std::string get_summary() const = 0;
    virtual std::string get_description() const = 0;
    virtual std::vector<std::string> get_files() const = 0;
    virtual bool is_installed() const = 0;
    virtual std::string get_from_repo_id() const = 0;
    virtual std::string get_repo_id() const = 0;
    virtual libdnf5::transaction::TransactionItemReason get_reason() const = 0;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_INTERFACES_PACKAGE_HPP
