/* includes; won't compile in tests, in the docs we leave out the comment lines to show them
#include <libdnf5/base/goal.hpp>
#include <libdnf5/repo/package_downloader.hpp>
*/

// Create a goal, which is a class that allows to add items for resolving into
// a transaction.
libdnf5::Goal goal(base);

// Add an RPM package named "one" for installation into the goal.
goal.add_rpm_install("one");

// Resolve the goal, create a transaction object.
//
// The argument is `allow_erasing`, a flag indicating whether to allow removing
// packages in the resolved transaction.
auto transaction = goal.resolve();

// We can iterate over the resolved transaction and inspect the packages.
std::cout << "Resolved transaction:" << std::endl;
for (const auto & tspkg : transaction.get_transaction_packages()) {
    std::cout << tspkg.get_package().get_nevra() << ": " << transaction_item_action_to_string(tspkg.get_action())
              << std::endl;
}

// This class demonstrates user-defined callbacks for the url/package downloads.
//
// The callbacks are implemented by inheriting from the callbacks base class
// and overriding its methods.
//
// We only override one of the callbacks here, see
// `libdnf5::repo::DownloadCallbacks` documentation for a complete list.
class PackageDownloadCallbacks : public libdnf5::repo::DownloadCallbacks {
private:
    int mirror_failure(
        [[maybe_unused]] void * user_cb_data,
        const char * msg,
        [[maybe_unused]] const char * url,
        [[maybe_unused]] const char * metadata) override {
        std::cout << "Mirror failure: " << msg << std::endl;
        return 0;
    }
};

base.set_download_callbacks(std::make_unique<PackageDownloadCallbacks>());

// Download the packages.
transaction.download();

// A class for defining the RPM transaction callbacks.
//
// Again, only a callback for when an RPM package installation starts, for a
// complete list of the callbacks see `libdnf5::rpm::TransactionCallbacks`
// documentation.
class TransactionCallbacks : public libdnf5::rpm::TransactionCallbacks {
    void install_start(const libdnf5::base::TransactionPackage & item, [[maybe_unused]] uint64_t total) override {
        std::cout << transaction_item_action_to_string(item.get_action()) << " " << item.get_package().get_nevra()
                  << std::endl;
    }
};

transaction.set_callbacks(std::make_unique<TransactionCallbacks>());

// Add transaction metadata to be stored in the history database.
transaction.set_description("install package one");

// Run the transaction.
std::cout << std::endl << "Running the transaction:" << std::endl;
transaction.run();
