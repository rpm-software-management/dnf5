// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/conf/option_string.hpp"

namespace libdnf5 {

class OptionString::Impl {
public:
    Impl(std::string && default_value) : icase(false), default_value(default_value), value(std::move(default_value)) {};
    Impl() : icase(false) {};
    Impl(std::string && regex, bool icase) : regex(std::move(regex)), icase(icase) {};
    Impl(std::string && default_value, std::string && regex, bool icase)
        : regex(std::move(regex)),
          icase(icase),
          default_value(default_value),
          value(std::move(default_value)) {};

    std::string regex;
    bool icase;
    std::string default_value;
    std::string value;
};


}  // namespace libdnf5
