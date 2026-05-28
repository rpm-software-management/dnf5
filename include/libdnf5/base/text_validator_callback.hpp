// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_BASE_TEXT_VALIDATOR_CALLBACK_HPP
#define LIBDNF5_BASE_TEXT_VALIDATOR_CALLBACK_HPP

#include "libdnf5/common/message.hpp"
#include "libdnf5/defs.h"

#include <string>

namespace libdnf5::base {

class TextValidator;

/// Input validator callback interface for text input validation.
/// Plugins can implement custom validators by inheriting from this class.
class LIBDNF_API TextValidatorCallback {
public:
    TextValidatorCallback();
    TextValidatorCallback(const TextValidatorCallback &) = delete;
    TextValidatorCallback & operator=(const TextValidatorCallback &) = delete;
    virtual ~TextValidatorCallback();

protected:
    /// Validate text input.
    /// @param input The text input to validate
    /// @return nullptr if the input is valid, or a pointer to a Message object with an error description if invalid.
    ///         The caller does NOT take ownership. The returned Message object must remain valid
    ///         until validate() is called again or the validator is destroyed.
    ///         Store the error Message as a member of the validator, not as a local variable:
    /// @code
    ///     class MyValidator : public TextValidatorCallback {
    ///         MyErrorMessage error_msg;   // a Message subclass stored as a member
    ///     protected:
    ///         libdnf5::Message * validate(const std::string & input) override {
    ///             if (input.empty()) return &error_msg;
    ///             return nullptr;
    ///         }
    ///     };
    /// @endcode
    virtual libdnf5::Message * validate(const std::string & input) = 0;

private:
    friend class TextValidator;
};

}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_TEXT_VALIDATOR_CALLBACK_HPP
