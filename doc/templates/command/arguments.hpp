#ifndef DNF5_COMMANDS_TEMPLATE_ARGUMENTS_HPP
#define DNF5_COMMANDS_TEMPLATE_ARGUMENTS_HPP


#include "utils/bgettext/bgettext-lib.h"

#include <libdnf-cli/session.hpp>


namespace dnf5 {

// This implementation is needed only if you are using unique_ptr as the type
// for the option of your command.
// You will have to initialize the Option yourself.
class BarOption : public libdnf::cli::session::BoolOption {
public:
    // Initialize the constructor passing command, long name, short name,
    // description and default value.
    explicit BarOption(libdnf::cli::session::Command & command)
        : BoolOption(command, "bar", '\0', _("print bar"), false) {}
};

}  // namespace dnf5


#endif  // DNF_COMMANDS_DOWNLOAD_TEMPLATE_ARGUMENTS_HPP
