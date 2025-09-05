/* includes; won't compile in tests, in the docs we leave out the comment lines to show them
# include <libdnf5/base/transaction_package.hpp>
# include <libdnf5/rpm/nevra.hpp>
# include <libdnf5/rpm/transaction_callbacks.hpp>
# include <libdnf5/transaction/transaction_item_action.hpp>
*/

class TransactionCallbacks : public libdnf5::rpm::TransactionCallbacks {
private:
    using tc = libdnf5::rpm::TransactionCallbacks;

    void install_stop(
        const libdnf5::base::TransactionPackage & item,
        [[maybe_unused]] uint64_t amount,
        [[maybe_unused]] uint64_t total) {
        if (item.get_action() == libdnf5::transaction::TransactionItemAction::INSTALL) {
            std::cout << "Installed: " << item.get_package().get_full_nevra() << std::endl;
        }
    }

    void script_start(
        [[maybe_unused]] const libdnf5::base::TransactionPackage * item,
        libdnf5::rpm::Nevra nevra,
        tc::ScriptType type) {
        std::cout << "Running " << tc::script_type_to_string(type)
                  << "scriptlet for:" << libdnf5::rpm::to_full_nevra_string(nevra) << std::endl;
    }

    void unpack_error(const libdnf5::base::TransactionPackage & item) {
        std::cout << "Unpack error: " << item.get_package().get_full_nevra() << std::endl;
    }
    void cpio_error(const libdnf5::base::TransactionPackage & item) {
        std::cout << "Cpio error: " << item.get_package().get_full_nevra() << std::endl;
    }
    void script_error(
        [[maybe_unused]] const libdnf5::base::TransactionPackage * item,
        libdnf5::rpm::Nevra nevra,
        tc::ScriptType type,
        uint64_t return_code) {
        std::cout << "Error in " << tc::script_type_to_string(type)
                  << " scriptlet: " << libdnf5::rpm::to_full_nevra_string(nevra) << " - return code " << return_code
                  << std::endl;
    }
};

// Set the transaction callbacks.
transaction.set_callbacks(std::make_unique<TransactionCallbacks>());

// Download the packages.
transaction.download();
