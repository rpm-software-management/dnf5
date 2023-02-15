# Create a goal, which is a class that allows to add items for resolving into
# a transaction.
goal = libdnf5.base.Goal(base)

# Add an RPM package named "one" for installation into the goal.
goal.add_rpm_install("one")

# Resolve the goal, create a transaction object.
#
# The argument is `allow_erasing`, a flag indicating wheter to allow removing
# packages in the resolved transaction.
transaction = goal.resolve()

# We can iterate over the resolved transction and inspect the packages.
print("Resolved transaction:")
for tspkg in transaction.get_transaction_packages():
    print(tspkg.get_package().get_nevra(), ": ",
          libdnf5.base.transaction.transaction_item_action_to_string(tspkg.get_action()))

# This class demonstrates user-defined callbacks for the package downloads.
#
# The callbacks are implemented by inheriting from the callbacks base class
# and overriding its methods.
#
# We only override one of the callbacks here, see
# `libdnf.repo.DownloadCallbacks` documentation for a complete list.
class PackageDownloadCallbacks(libdnf5.repo.DownloadCallbacks):
    def mirror_failure(self, user_cb_data, msg, url=""):
        print("Mirror failure: ", msg)
        return 0

# Create a package downloader.
downloader = libdnf5.repo.PackageDownloader()

downloader_callbacks = PackageDownloadCallbacks()
base.set_download_callbacks(libdnf5.repo.DownloadCallbacksUniquePtr(downloader_callbacks))

# Download the packages.
transaction.download()

# A class for defining the RPM transaction callbacks.
#
# Again, only a callback for when an RPM package installation starts, for a
# complete list of the callbacks see `libdnf.rpm.TransactionCallbacks`
# documentation.
class TransactionCallbacks(libdnf5.rpm.TransactionCallbacks):
    def install_start(self, item, total=0):
        print(libdnf5.base.transaction.transaction_item_action_to_string(item.get_action()), " ",
              item.get_package().get_nevra())
# Run the transaction.
#
# The second through fourth arguments are transaction metadata that will be
# stored in the history database.
#
# The second argument is expected to be a verbose description of the
# transaction. The third argument is user_id, omitted here for simplicity. The
# fourth argument can be an arbitrary user comment.
print("Running the transaction:")
transaction_callbacks = TransactionCallbacks()
transaction_callbacks_ptr = libdnf5.rpm.TransactionCallbacksUniquePtr(transaction_callbacks)

transaction.run(transaction_callbacks_ptr, "install package one")
