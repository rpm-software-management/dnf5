// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_ADAPTERS_MODULE_HPP
#define LIBDNF5_CLI_OUTPUT_ADAPTERS_MODULE_HPP

#include "module_tmpl.hpp"

#include "libdnf5-cli/defs.h"

#include <libdnf5/module/module_dependency.hpp>
#include <libdnf5/module/module_item.hpp>
#include <libdnf5/module/module_profile.hpp>

namespace libdnf5::cli::output {

extern template class LIBDNF_CLI_API ModuleDependencyAdapter<libdnf5::module::ModuleDependency>;
extern template class LIBDNF_CLI_API ModuleItemAdapter<libdnf5::module::ModuleItem>;
extern template class LIBDNF_CLI_API ModuleProfileAdapter<libdnf5::module::ModuleProfile>;

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_ADAPTERS_MODULE_HPP
