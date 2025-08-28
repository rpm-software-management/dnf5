// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DNF5_PLUGINS_AUTOMATIC_PLUGIN_TRANSACTION_CALLBACKS_SIMPLE_HPP
#define DNF5_PLUGINS_AUTOMATIC_PLUGIN_TRANSACTION_CALLBACKS_SIMPLE_HPP

#include <dnf5/context.hpp>
#include <libdnf5/rpm/transaction_callbacks.hpp>

#include <sstream>

namespace dnf5 {

/// Simple callbacks class. It does not print any progressbars, only
/// the rpm transaction error messages.
class TransactionCallbacksSimple : public libdnf5::rpm::TransactionCallbacks {
public:
    explicit TransactionCallbacksSimple(Context & context, std::stringstream & output_stream)
        : context(context),
          output_stream(output_stream) {}

    void transaction_start(uint64_t total) override;
    void install_start(const libdnf5::base::TransactionPackage & item, uint64_t total) override;
    void uninstall_start(const libdnf5::base::TransactionPackage & item, uint64_t total) override;
    void unpack_error(const libdnf5::base::TransactionPackage & item) override;
    void cpio_error(const libdnf5::base::TransactionPackage & item) override;
    void script_error(
        [[maybe_unused]] const libdnf5::base::TransactionPackage * item,
        libdnf5::rpm::Nevra nevra,
        libdnf5::rpm::TransactionCallbacks::ScriptType type,
        uint64_t return_code) override;
    void script_start(
        [[maybe_unused]] const libdnf5::base::TransactionPackage * item,
        libdnf5::rpm::Nevra nevra,
        libdnf5::rpm::TransactionCallbacks::ScriptType type) override;
    void script_stop(
        [[maybe_unused]] const libdnf5::base::TransactionPackage * item,
        libdnf5::rpm::Nevra nevra,
        libdnf5::rpm::TransactionCallbacks::ScriptType type,
        [[maybe_unused]] uint64_t return_code) override;


private:
    Context & context;
    std::stringstream & output_stream;
};

}  // namespace dnf5

#endif
