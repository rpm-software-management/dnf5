/* includes; won't compile in tests, in the docs we leave out the comment lines to show them
#include <libdnf/base/goal.hpp>
#include <libdnf/repo/package_downloader.hpp>
*/

// Create a goal, which is a class that allows to add items for resolving into
// a transaction.
libdnf::Goal goal(base);

// Add an RPM package named "one" for installation into the goal.
goal.add_rpm_install("one");

// Resolve the goal, create a transaction object.
//
// The argument is `allow_erasing`, a flag indicating wheter to allow removing
// packages in the resolved transaction.
auto transaction = goal.resolve(false);

// We can iterate over the resolved transction and inspect the packages.
std::cout << "Resolved transaction:" << std::endl;
for (const auto & tspkg : transaction.get_transaction_packages()) {
    std::cout
        << tspkg.get_package().get_nevra() << ": "
        << transaction_item_action_to_string(tspkg.get_action()) << std::endl;
}

// This class demonstrates user-defined callbacks for the package downloads.
//
// The callbacks are implemented by inheriting from the callbacks base class
// and overriding its methods.
//
// We only override one of the callbacks here, see
// `libdnf::repo::DownloadCallbacks` documentation for a complete list.
class PackageDownloadCallbacks : public libdnf::repo::DownloadCallbacks {
    int mirror_failure(const char * msg, [[maybe_unused]] const char * url) override {
        std::cout << "Mirror failure: " << msg << std::endl;
        return 0;
    }
};

// Create a package downloader.
libdnf::repo::PackageDownloader downloader;

// Add the inbound packages (packages that are being installed on the system)
// to the downloader.
for (auto & tspkg : transaction.get_transaction_packages()) {
    if (transaction_item_action_is_inbound(tspkg.get_action())) {
        downloader.add(
            tspkg.get_package(),
            std::make_unique<PackageDownloadCallbacks>()
        );
    }
}

// Download the packages.
//
// The first argument is `fail_fast`, meaning the download will fail right away
// on a first package download failure. The second argument is `resume`, if
// `true`, the downloader will try to resume downloads of any partially
// downloaded RPMs.
downloader.download(true, true);

// A class for defining the RPM transaction callbacks.
//
// Again, only a callback for when an RPM package installation starts, for a
// complete list of the callbacks see `libdnf::rpm::TransactionCallbacks`
// documentation.
class TransactionCallbacks : public libdnf::rpm::TransactionCallbacks {
    void install_start(
        const libdnf::rpm::TransactionItem & item,
        [[maybe_unused]] uint64_t total) override
    {
        std::cout
            << transaction_item_action_to_string(item.get_action()) << " "
            << item.get_package().get_nevra() << std::endl;
    }
};

// Run the transaction.
//
// The second through fourth arguments are transaction metadata that will be
// stored in the history database.
//
// The second argument is expected to be a verbose description of the
// transaction. The third argument is user_id, omitted here for simplicity. The
// fourth argument can be an arbitrary user comment.
std::cout << std::endl << "Running the transaction:" << std::endl;
transaction.run(
    std::make_unique<TransactionCallbacks>(),
    "install package one",
    std::nullopt,
    std::nullopt
);
