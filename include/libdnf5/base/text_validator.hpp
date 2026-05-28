// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_BASE_TEXT_VALIDATOR_HPP
#define LIBDNF5_BASE_TEXT_VALIDATOR_HPP

#include "libdnf5/common/message.hpp"
#include "libdnf5/defs.h"

#include <string>

namespace libdnf5 {
class Base;
}

namespace libdnf5::base {

class TextValidatorCallback;

/// Created by Base, wrapping the caller's TextValidatorCallback.
/// Overrides must not construct this class directly.
class LIBDNF_API TextValidator {
public:
    TextValidator() = delete;
    TextValidator(const TextValidator &) = delete;
    TextValidator(TextValidator &&) = delete;
    TextValidator & operator=(const TextValidator &) = delete;
    TextValidator & operator=(TextValidator &&) = delete;

    /// Validate text input by delegating to the underlying TextValidatorCallback.
    /// @return nullptr if the input is valid, or a Message with an error description if invalid.
    libdnf5::Message * validate(const std::string & input);

private:
    friend class ::libdnf5::Base;
    explicit TextValidator(TextValidatorCallback * impl);
    ~TextValidator();
    TextValidatorCallback * impl;
};

}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_TEXT_VALIDATOR_HPP
