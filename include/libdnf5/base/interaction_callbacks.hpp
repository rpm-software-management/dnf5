// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_BASE_INTERACTION_CALLBACKS_HPP
#define LIBDNF5_BASE_INTERACTION_CALLBACKS_HPP

#include "base_weak.hpp"

#include "libdnf5/base/text_validator.hpp"
#include "libdnf5/common/message.hpp"
#include "libdnf5/defs.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace libdnf5::base {

/// Return value constants for InteractionCallbacks confirm(), choice(), and input_text() methods.
#ifdef SWIG
/* clang-format off */
#define constexpr %constant
/* clang-format on */
#endif
constexpr int32_t ANSWER_YES = -1;      ///< User confirmed / a string entered
constexpr int32_t ANSWER_NO = -2;       ///< User denied / undefined string
constexpr int32_t ANSWER_DEFAULT = -3;  ///< Use the caller-supplied default
constexpr int32_t ANSWER_ABORT = -4;    ///< Abort the operation (user request or I/O error)
#ifdef SWIG
#undef constexpr
#endif

/// Base class for interaction callbacks.
/// To implement callbacks, inherit from this class and override virtual methods.
///
/// These callbacks allow libdnf5 plugins to interact with the user through the application.
/// The application implements these methods to provide user interface
/// for messages, confirmations, choices, text input, and progress.
class LIBDNF_API InteractionCallbacks {
public:
    enum class MessageLevel : unsigned int { ERROR = 1, WARNING, NOTICE, INFO };
    enum class ProgressState { NEW, UPDATE, END_OK, END_ERROR, INFO, WARNING, ERROR };

    explicit InteractionCallbacks();
    InteractionCallbacks(const InteractionCallbacks &) = delete;
    InteractionCallbacks(InteractionCallbacks &&) = delete;
    virtual ~InteractionCallbacks();

    InteractionCallbacks & operator=(const InteractionCallbacks &) = delete;
    InteractionCallbacks & operator=(InteractionCallbacks &&) = delete;

protected:
    /// @return The `Base` object to which this object belongs.
    libdnf5::BaseWeakPtr get_base() const;

    /// Override to display a message to the user. Called via Base::message().
    /// @param level The message level (ERROR, WARNING, NOTICE, INFO)
    /// @param message The message to display
    virtual void message(MessageLevel level, const libdnf5::Message & msg);

    /// Override to ask the user to confirm an action. Called via Base::confirm().
    /// The default implementation returns `ANSWER_NO` if `assumeno` is set, `ANSWER_YES` if `assumeyes`
    /// is set, and `ANSWER_DEFAULT` otherwise.
    /// @param message The confirmation message describing the action
    /// @param default_answer The answer preferred by the caller, used when neither `assumeyes`
    ///        nor `assumeno` configuration option is active
    /// @return `ANSWER_YES` (-1) to confirm, `ANSWER_NO` (-2) to deny, `ANSWER_DEFAULT` (-3) to use caller default,
    ///         `ANSWER_ABORT` (-4) to abort the operation
    virtual int32_t confirm(const libdnf5::Message & msg, bool default_answer);

    /// Override to ask the user to select an option. Called via Base::choice().
    /// @param msg The message describing the choice
    /// @param options Vector of pointers to available option messages (for translation support)
    /// @param default_option Index of the option preferred by the caller (0-based)
    /// @return Index of the selected choice (0-based), `ANSWER_DEFAULT` (-3) to use caller default,
    ///         or `ANSWER_ABORT` (-4) to abort the operation
    virtual int32_t choice(
        const libdnf5::Message & msg, const std::vector<libdnf5::Message *> & options, int32_t default_option);

    /// Override to ask the user for text input. Called via Base::input_text().
    /// The implementation should repeatedly prompt the user until valid input is provided
    /// or the user cancels. If a validator is provided, call validator->validate() on each
    /// input and display any error message before prompting again.
    /// @param out_text Output parameter - filled with user input when returning `ANSWER_YES` or `ANSWER_NO`
    /// @param msg The message describing what input is needed
    /// @param default_text Optional pre-filled text offered to the user, or nullptr (default is undefined text).
    /// @param validator Optional validator for checking input validity, or nullptr to accept any input.
    /// @return `ANSWER_YES` (-1) if a string was entered (out_text filled; may be empty/zero-length),
    ///         `ANSWER_NO` (-2) if no string was returned (out_text unchanged; analogous to NULL char*),
    ///         `ANSWER_DEFAULT` (-3) if the caller should use default_text (out_text unchanged),
    ///         `ANSWER_ABORT` (-4) to abort the operation (out_text unchanged)
    virtual int32_t input_text(
        std::string & out_text,
        const libdnf5::Message & msg,
        const char * default_text,
        libdnf5::base::TextValidator * validator);

    /// Override to report progress. Called via Base::progress().
    /// On NEW the implementation allocates a handle and returns it; the caller
    /// passes the same handle back in all subsequent calls for that entry.
    /// The handle is an opaque integer token whose meaning is defined solely
    /// by the implementation; the caller treats it as an integer number.
    /// @param handle  For NEW: ignored. For other states: the handle returned
    ///                by the corresponding NEW call.
    /// @param state   The progress state
    /// @param msg     Optional description of current activity, or nullptr
    /// @param count   Number of completed items
    /// @param total   Total number of items
    /// @return For NEW: the handle of the new progress entry.
    ///         For other states: >= 0 to continue, < 0 to request cancellation.
    /// The default implementation returns 0 for NEW and returns the handle
    /// unchanged for all other states without performing any action.
    virtual int progress(int handle, ProgressState state, const libdnf5::Message * msg, int64_t count, int64_t total);

private:
    friend class ::libdnf5::Base;

    /// Called by Base::set_interaction_callbacks() to associate this object with a Base instance.
    void register_base(Base & base);

    /// Weak pointer to the Base object that owns this callbacks instance.
    libdnf5::BaseWeakPtr base;
};

}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_INTERACTION_CALLBACKS_HPP
