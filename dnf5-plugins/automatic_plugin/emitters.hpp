/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef DNF5_PLUGINS_AUTOMATIC_PLUGIN_EMITTERS_HPP
#define DNF5_PLUGINS_AUTOMATIC_PLUGIN_EMITTERS_HPP

#include "config_automatic.hpp"

#include <libdnf5/base/transaction.hpp>

#include <sstream>

namespace dnf5 {


class Emitter {
public:
    Emitter(
        const ConfigAutomatic & config_automatic,
        const libdnf5::base::Transaction & transaction,
        const std::stringstream & output_stream,
        const bool success)
        : config_automatic(config_automatic),
          transaction(transaction),
          output_stream(output_stream),
          success(success) {}

    /// Notify the user about the status of dnf-automatic run.
    virtual void notify() = 0;

    /// Return short message containing basic information about automatic upgrade.
    std::string short_message();

protected:
    // dnf automatic configuration
    const ConfigAutomatic & config_automatic;
    // resolved upgrade transaction
    const libdnf5::base::Transaction & transaction;
    // stream with captured upgrade outputs
    const std::stringstream & output_stream;
    const bool success;

    /// Return number of available upgrades.
    int upgrades_count();
};

/// Print the results to standard output.
class EmitterStdIO : public Emitter {
public:
    using Emitter::Emitter;
    EmitterStdIO() = delete;

    void notify() override;
};

/// Send the results to /etc/motd.d/dnf-automatic file
class EmitterMotd : public Emitter {
public:
    using Emitter::Emitter;
    EmitterMotd() = delete;

    void notify() override;
};


/// Send the results using the shell command
class EmitterCommand : public Emitter {
public:
    using Emitter::Emitter;
    EmitterCommand() = delete;

    void notify() override;
};

/// Send the results via email using the shell command
class EmitterCommandEmail : public Emitter {
public:
    using Emitter::Emitter;
    EmitterCommandEmail() = delete;

    void notify() override;
};

/// Send the results via email
class EmitterEmail : public Emitter {
public:
    using Emitter::Emitter;
    EmitterEmail() = delete;

    void notify() override;
};

}  // namespace dnf5


#endif  // DNF5_PLUGINS_AUTOMATIC_PLUGIN_EMITTERS_HPP
