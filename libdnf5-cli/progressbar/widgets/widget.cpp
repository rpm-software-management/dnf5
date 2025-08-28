// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "libdnf5-cli/progressbar/widgets/widget.hpp"

#include <iostream>
#include <string>


namespace libdnf5::cli::progressbar {


std::ostream & operator<<(std::ostream & stream, Widget & widget) {
    stream << widget.to_string();
    return stream;
}


}  // namespace libdnf5::cli::progressbar
