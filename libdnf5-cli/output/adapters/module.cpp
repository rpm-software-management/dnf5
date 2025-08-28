// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "libdnf5-cli/output/adapters/module.hpp"

namespace libdnf5::cli::output {

template class ModuleDependencyAdapter<libdnf5::module::ModuleDependency>;
template class ModuleItemAdapter<libdnf5::module::ModuleItem>;
template class ModuleProfileAdapter<libdnf5::module::ModuleProfile>;

}  // namespace libdnf5::cli::output
