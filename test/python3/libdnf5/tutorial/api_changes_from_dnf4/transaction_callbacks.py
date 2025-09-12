class TransactionCallbacks(libdnf5.rpm.TransactionCallbacks):
    def install_stop(self, item, amount, total):
        if item.get_action() == libdnf5.transaction.TransactionItemAction_INSTALL:
            print(f"Installed: {item.get_package().get_full_nevra()}")

    def script_start(self, item, nevra, type):
        script = libdnf5.rpm.TransactionCallbacks.script_type_to_string(type)
        nevra_str = libdnf5.rpm.to_full_nevra_string(nevra)
        print(f"Running {script} scriptlet for: {nevra_str}")

    def unpack_error(self, item):
        print(f"Unpack error: {item.get_package().get_full_nevra()}")

    def cpio_error(self, item):
        print(f"Cpio error: {item.get_package().get_full_nevra()}")

    def script_error(self, item, nevra, type, return_code):
        script = libdnf5.rpm.TransactionCallbacks.script_type_to_string(type)
        nevra_str = libdnf5.rpm.to_full_nevra_string(nevra)
        print(f"Error in {script} scriptlet: {nevra_str} - return code {return_code}")


# Set the transaction callbacks.
transaction_callbacks = TransactionCallbacks()
transaction.set_callbacks(
    libdnf5.rpm.TransactionCallbacksUniquePtr(transaction_callbacks))

# Run the transaction.
transaction.run()
