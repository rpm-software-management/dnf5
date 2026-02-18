#ifndef DNF5_COMMANDS_TEMPLATE_ARGUMENTS_HPP
#define DNF5_COMMANDS_TEMPLATE_ARGUMENTS_HPP

#include <libdnf5-cli/session.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>

namespace dnf5 {

// This implementation is needed only if you are using unique_ptr as the type
// for the option of your command.
// You will have to initialize the Option yourself.
class BarOption : public libdnf5::cli::session::BoolOption {
public:
    // Initialize the constructor passing command, long name, short name,
    // description and default value.
    explicit BarOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "bar", '\0', _("print bar"), false) {}
};

}  // namespace dnf5

#endif  // DNF_COMMANDS_TEMPLATE_ARGUMENTS_HPP
