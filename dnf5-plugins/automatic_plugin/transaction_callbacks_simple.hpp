/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DNF5_PLUGINS_AUTOMATIC_PLUGIN_TRANSACTION_CALLBACKS_SIMPLE_HPP
#define DNF5_PLUGINS_AUTOMATIC_PLUGIN_TRANSACTION_CALLBACKS_SIMPLE_HPP

#include <libdnf5/rpm/transaction_callbacks.hpp>

#include <sstream>

namespace dnf5 {

/// Simple callbacks class. It does not print any progressbars, only
/// the rpm transaction error messages.
class TransactionCallbacksSimple : public libdnf5::rpm::TransactionCallbacks {
public:
    explicit TransactionCallbacksSimple(std::stringstream & output_stream) : output_stream(output_stream) {}

    void transaction_start(uint64_t total) override;
    void install_start(const libdnf5::rpm::TransactionItem & item, uint64_t total) override;
    void uninstall_start(const libdnf5::rpm::TransactionItem & item, uint64_t total) override;
    void unpack_error(const libdnf5::rpm::TransactionItem & item) override;
    void cpio_error(const libdnf5::rpm::TransactionItem & item) override;
    void script_error(
        [[maybe_unused]] const libdnf5::rpm::TransactionItem * item,
        libdnf5::rpm::Nevra nevra,
        libdnf5::rpm::TransactionCallbacks::ScriptType type,
        uint64_t return_code) override;
    void script_start(
        [[maybe_unused]] const libdnf5::rpm::TransactionItem * item,
        libdnf5::rpm::Nevra nevra,
        libdnf5::rpm::TransactionCallbacks::ScriptType type) override;
    void script_stop(
        [[maybe_unused]] const libdnf5::rpm::TransactionItem * item,
        libdnf5::rpm::Nevra nevra,
        libdnf5::rpm::TransactionCallbacks::ScriptType type,
        [[maybe_unused]] uint64_t return_code) override;


private:
    std::stringstream & output_stream;
};

}  // namespace dnf5

#endif
