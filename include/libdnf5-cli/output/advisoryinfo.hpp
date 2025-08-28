// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_ADVISORYINFO_HPP
#define LIBDNF5_CLI_OUTPUT_ADVISORYINFO_HPP

#include "interfaces/advisory.hpp"

#include "libdnf5-cli/defs.h"

namespace libdnf5::cli::output {

class LIBDNF_CLI_API AdvisoryInfo {
public:
    AdvisoryInfo();
    ~AdvisoryInfo();

    void add_advisory(IAdvisory & advisory);
    void print();

private:
    class LIBDNF_CLI_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

class LIBDNF_CLI_API AdvisoryInfoJSON {
public:
    AdvisoryInfoJSON();
    ~AdvisoryInfoJSON();

    void add_advisory(IAdvisory & advisory);
    void print();

private:
    class LIBDNF_CLI_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_ADVISORYLIST_HPP
