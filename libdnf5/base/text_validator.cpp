// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/base/text_validator.hpp"

#include "libdnf5/base/text_validator_callback.hpp"

namespace libdnf5::base {

TextValidator::TextValidator(TextValidatorCallback * impl) : impl(impl) {}

TextValidator::~TextValidator() = default;

libdnf5::Message * TextValidator::validate(const std::string & input) {
    return impl ? impl->validate(input) : nullptr;
}

}  // namespace libdnf5::base
